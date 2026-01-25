#include "codegen/codegen.h"
#include "parser/parser.h"
#include "plugins/plugin_manager.h"
#include "repl/repl.h"
#include "zen/zen_facts.h"
#include "zprep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <stdarg.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>
#endif

// Forward decl for LSP
int lsp_main(int argc, char **argv);

void print_search_paths()
{
    printf("Search paths:\n");
    printf("  ./\n");
    printf("  ./std/\n");
    printf("  /usr/local/share/zenc\n");
    printf("  /usr/share/zenc\n");
}

void print_version()
{
    printf("Zen C version %s\n", ZEN_VERSION);
}

void print_usage()
{
    printf("Usage: zc [command] [options] <file.zc>\n");
    printf("Commands:\n");
    printf("  run     Compile and run the program\n");
    printf("  build   Compile to executable\n");
    printf("  check   Check for errors only\n");
    printf("  repl    Start Interactive REPL\n");
    printf("  transpile Transpile to C code only (no compilation)\n");
    printf("  lsp     Start Language Server\n");
    printf("Options:\n");
    printf("  --version       Print version information\n");
    printf("  -o <file>       Output executable name\n");
    printf("  --emit-c        Keep generated C file (out.c)\n");
    printf("  --freestanding  Freestanding mode (no stdlib)\n");
    printf("  --cc <compiler> C compiler to use (gcc, clang, tcc, zig, zig++)\n");
    printf("  -O<level>       Optimization level\n");
    printf("  -g              Debug info\n");
    printf("  -v, --verbose   Verbose output\n");
    printf("  -q, --quiet     Quiet output\n");
    printf("  --no-zen        Disable Zen facts\n");
    printf("  -c              Compile only (produce .o)\n");
    printf("  --cpp           Use C++ mode.\n");
    printf("  --cuda          Use CUDA mode (requires nvcc).\n");
#if defined(_WIN32)
    printf("  --msvc              (Windows + --cpp) Use MSVC ABI/headers/libs with zig c++\n");
    printf("  --msvc-root <p>     Override MSVC root (…\\VC\\Tools\\MSVC\\<ver>)\n");
    printf("  --win-kits-root <p> Override Windows Kits root (…\\Windows Kits\\10)\n");
    printf("  --win-sdk-ver <v>   Override Windows SDK version (eg 10.0.26100.0)\n");
#endif
}

#if defined(_WIN32)
static void appendf(char *dst, size_t cap, const char *fmt, ...)
{
    size_t len = strlen(dst);
    if (len >= cap)
        return;

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(dst + len, cap - len, fmt, ap);
    va_end(ap);
}

static void zc_add_msvc_toolchain_flags(
    char *cflags, size_t cflags_cap,
    char *linkflags, size_t linkflags_cap,
    const char *msvc_root,
    const char *win_kits_root,
    const char *win_sdk_ver)
{
    // MSVC / Windows SDK computed paths (x64)
    char msvc_inc[512];
    char msvc_lib[512];
    char sdk_inc_um[512];
    char sdk_inc_shared[512];
    char sdk_inc_ucrt[512];
    char sdk_inc_winrt[512];
    char sdk_lib_um[512];
    char sdk_lib_ucrt[512];

    snprintf(msvc_inc, sizeof(msvc_inc), "%s\\include", msvc_root);
    snprintf(msvc_lib, sizeof(msvc_lib), "%s\\lib\\x64", msvc_root);

    snprintf(sdk_inc_um, sizeof(sdk_inc_um), "%s\\Include\\%s\\um", win_kits_root, win_sdk_ver);
    snprintf(sdk_inc_shared, sizeof(sdk_inc_shared), "%s\\Include\\%s\\shared", win_kits_root, win_sdk_ver);
    snprintf(sdk_inc_ucrt, sizeof(sdk_inc_ucrt), "%s\\Include\\%s\\ucrt", win_kits_root, win_sdk_ver);
    snprintf(sdk_inc_winrt, sizeof(sdk_inc_winrt), "%s\\Include\\%s\\winrt", win_kits_root, win_sdk_ver);

    snprintf(sdk_lib_um, sizeof(sdk_lib_um), "%s\\Lib\\%s\\um\\x64", win_kits_root, win_sdk_ver);
    snprintf(sdk_lib_ucrt, sizeof(sdk_lib_ucrt), "%s\\Lib\\%s\\ucrt\\x64", win_kits_root, win_sdk_ver);

    // ---- Compiler flags ----
    appendf(cflags, cflags_cap, " -target x86_64-windows-msvc");
    appendf(cflags, cflags_cap, " -std=c++20");
    appendf(cflags, cflags_cap, " -nostdinc++");

    // Force /MD-style CRT selection for MSVC builds (use DLL CRT)
    appendf(cflags, cflags_cap, " -D_DLL");

    appendf(cflags, cflags_cap, " -isystem \"%s\"", msvc_inc);
    appendf(cflags, cflags_cap, " -isystem \"%s\"", sdk_inc_um);
    appendf(cflags, cflags_cap, " -isystem \"%s\"", sdk_inc_shared);
    appendf(cflags, cflags_cap, " -isystem \"%s\"", sdk_inc_ucrt);
    appendf(cflags, cflags_cap, " -isystem \"%s\"", sdk_inc_winrt);

    // ---- Linker flags ----
    appendf(linkflags, linkflags_cap, " -L\"%s\"", msvc_lib);
    appendf(linkflags, linkflags_cap, " -L\"%s\"", sdk_lib_um);
    appendf(linkflags, linkflags_cap, " -L\"%s\"", sdk_lib_ucrt);

    // IMPORTANT:
    // Do NOT explicitly link libvcruntime/libucrt here (those are the static CRT libs),
    // because it can conflict with the DLL import libs that lld-link/zig pulls in for /MD.
    // Only link the C++ STL import lib + legacy stdio helpers; the rest is resolved via defaults.
    appendf(linkflags, linkflags_cap, " -lmsvcprt -llegacy_stdio_definitions");

    // Core Win32 libs
    appendf(linkflags, linkflags_cap, " -lkernel32 -luser32 -ladvapi32 -lntdll");
}
#endif

int main(int argc, char **argv)
{
    memset(&g_config, 0, sizeof(g_config));

#if defined(_WIN32)
    strcpy(g_config.cc, "zig cc");
#else
    strcpy(g_config.cc, "gcc");
#endif

#if defined(_WIN32)
    int use_msvc_toolchain = 0;

    // Defaults (override via flags)
    const char *msvc_root = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Tools\\MSVC\\14.44.35207";
    const char *win_kits_root = "C:\\Program Files (x86)\\Windows Kits\\10";
    const char *win_sdk_ver = "10.0.26100.0";
#endif

    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    // Parse command
    char *command = argv[1];
    int arg_start = 2;

    if (strcmp(command, "lsp") == 0)
    {
        return lsp_main(argc, argv);
    }
    else if (strcmp(command, "repl") == 0)
    {
        run_repl(argv[0]); // Pass self path for recursive calls
        return 0;
    }
    else if (strcmp(command, "transpile") == 0)
    {
        g_config.mode_transpile = 1;
        g_config.emit_c = 1; // Transpile implies emitting C
    }
    else if (strcmp(command, "run") == 0)
    {
        g_config.mode_run = 1;
    }
    else if (strcmp(command, "check") == 0)
    {
        g_config.mode_check = 1;
    }
    else if (strcmp(command, "build") == 0)
    {
        // default mode
    }
    else if (command[0] == '-')
    {
        // implicit build or run? assume build if starts with flag, but usually
        // command first If file provided directly: "zc file.zc" -> build
        if (strchr(command, '.'))
        {
            // treat as filename
            g_config.input_file = command;
            arg_start = 2; // already consumed
        }
        else
        {
            // Flags
            arg_start = 1;
        }
    }
    else
    {
        // Check if file
        if (strchr(command, '.'))
        {
            g_config.input_file = command;
            arg_start = 2;
        }
    }

    // Parse args
    for (int i = arg_start; i < argc; i++)
    {
        char *arg = argv[i];
        if (strcmp(arg, "--emit-c") == 0)
        {
            g_config.emit_c = 1;
        }
        else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-V") == 0)
        {
            print_version();
            return 0;
        }
        else if (strcmp(arg, "--verbose") == 0 || strcmp(arg, "-v") == 0)
        {
            g_config.verbose = 1;
        }
        else if (strcmp(arg, "--quiet") == 0 || strcmp(arg, "-q") == 0)
        {
            g_config.quiet = 1;
        }
        else if (strcmp(arg, "--no-zen") == 0)
        {
            g_config.no_zen = 1;
        }
        else if (strcmp(arg, "--freestanding") == 0)
        {
            g_config.is_freestanding = 1;
        }
        else if (strcmp(arg, "--cpp") == 0)
        {
#if defined(_WIN32)
            strcpy(g_config.cc, "zig c++");
#else
            strcpy(g_config.cc, "g++");
#endif
            g_config.use_cpp = 1;
        }
        else if (strcmp(arg, "--cuda") == 0)
        {
            strcpy(g_config.cc, "nvcc");
            g_config.use_cuda = 1;
            g_config.use_cpp = 1; // CUDA implies C++ mode.
        }
        else if (strcmp(arg, "--check") == 0)
        {
            g_config.mode_check = 1;
        }
        else if (strcmp(arg, "--cc") == 0)
        {
            if (i + 1 < argc)
            {
                char *cc_arg = argv[++i];
                // Handle "zig" shorthand for "zig cc"
                if (strcmp(cc_arg, "zig") == 0)
                {
                    strcpy(g_config.cc, "zig cc");
                }
                else if (strcmp(cc_arg, "zig++") == 0)
                {
                    strcpy(g_config.cc, "zig c++");
                }
                else
                {
                    strcpy(g_config.cc, cc_arg);
                }
            }
        }
        else if (strcmp(arg, "-o") == 0)
        {
            if (i + 1 < argc)
            {
                g_config.output_file = argv[++i];
            }
        }
        else if (strncmp(arg, "-O", 2) == 0)
        {
            // Add to CFLAGS
            strcat(g_config.gcc_flags, " ");
            strcat(g_config.gcc_flags, arg);
        }
        else if (strcmp(arg, "-g") == 0)
        {
            strcat(g_config.gcc_flags, " -g");
        }
#if defined(_WIN32)
        else if (strcmp(arg, "--msvc") == 0)
        {
            use_msvc_toolchain = 1;
        }
        else if (strcmp(arg, "--msvc-root") == 0 && i + 1 < argc)
        {
            msvc_root = argv[++i];
        }
        else if (strcmp(arg, "--win-kits-root") == 0 && i + 1 < argc)
        {
            win_kits_root = argv[++i];
        }
        else if (strcmp(arg, "--win-sdk-ver") == 0 && i + 1 < argc)
        {
            win_sdk_ver = argv[++i];
        }
#endif
        else if (arg[0] == '-')
        {
            // Unknown flag or C flag
            strcat(g_config.gcc_flags, " ");
            strcat(g_config.gcc_flags, arg);
        }
        else
        {
            if (!g_config.input_file)
            {
                g_config.input_file = arg;
            }
            else
            {
                printf("Multiple input files not supported yet.\n");
                return 1;
            }
        }
    }

    if (!g_config.input_file)
    {
        printf("Error: No input file specified.\n");
        return 1;
    }

    g_current_filename = g_config.input_file;

    // Load file
    char *src = load_file(g_config.input_file);
    if (!src)
    {
        printf("Error: Could not read file %s\n", g_config.input_file);
        return 1;
    }

    init_builtins();
    zen_init();

    // Initialize Plugin Manager
    zptr_plugin_mgr_init();

    // Parse context init
    ParserContext ctx;
    memset(&ctx, 0, sizeof(ctx));

    // Scan for build directives (e.g. //> link: -lm)
    scan_build_directives(&ctx, src);

    Lexer l;
    lexer_init(&l, src);

    ctx.hoist_out = tmpfile(); // Temp file for plugin hoisting
    if (!ctx.hoist_out)
    {
        perror("tmpfile for hoisting");
        return 1;
    }
    g_parser_ctx = &ctx;

    if (!g_config.quiet)
    {
        printf("[zc] Compiling %s...\n", g_config.input_file);
    }

    ASTNode *root = parse_program(&ctx, &l);
    if (!root)
    {
        // Parse failed
        return 1;
    }

    if (!validate_types(&ctx))
    {
        // Type validation failed
        return 1;
    }

    if (g_config.mode_check)
    {
        // Just verify
        printf("Check passed.\n");
        return 0;
    }

    // Determine temporary filename based on mode
    const char *temp_source_file = "out.c";
    if (g_config.use_cuda)
    {
        temp_source_file = "out.cu";
    }
    else if (g_config.use_cpp)
    {
        temp_source_file = "out.cpp";
    }

    // Codegen to C/C++/CUDA
    FILE *out = fopen(temp_source_file, "w");
    if (!out)
    {
        perror("fopen temp output");
        return 1;
    }

    codegen_node(&ctx, root, out);
    fclose(out);

    if (g_config.mode_transpile)
    {
        if (g_config.output_file)
        {
            // If user specified -o, rename temp file to that
            if (rename(temp_source_file, g_config.output_file) != 0)
            {
                perror("rename output");
                return 1;
            }
            if (!g_config.quiet)
            {
                printf("[zc] Transpiled to %s\n", g_config.output_file);
            }
        }
        else
        {
            if (!g_config.quiet)
            {
                printf("[zc] Transpiled to %s\n", temp_source_file);
            }
        }
        // Done, no C compilation
        return 0;
    }

#if defined(_WIN32)
    // Apply MSVC toolchain flags if requested and we're in C++ mode.
    // Intended for: zig c++ + prebuilt MSVC-ABI libraries (eg Panda3D SDK).
    if (use_msvc_toolchain && g_config.use_cpp)
    {
        zc_add_msvc_toolchain_flags(
            g_config.gcc_flags, sizeof(g_config.gcc_flags),
            g_link_flags, sizeof(g_link_flags),
            msvc_root, win_kits_root, win_sdk_ver);
    }
#endif

    // Compile C
    char cmd[8192];
#if defined(_WIN32)
    const char *outfile = g_config.output_file ? g_config.output_file : "a.exe";
#else
    const char *outfile = g_config.output_file ? g_config.output_file : "a.out";
#endif

#if defined(_WIN32)
    snprintf(cmd, sizeof(cmd),
             "%s %s %s %s -o \"%s\" \"%s\" -I\"./src\" %s",
             g_config.cc,
             g_config.gcc_flags,
             g_cflags,
             g_config.is_freestanding ? "-ffreestanding" : "",
             outfile,
             temp_source_file,
             g_link_flags);
#else
    snprintf(cmd, sizeof(cmd),
             "%s %s %s %s -o \"%s\" \"%s\" -lm %s -I\"./src\" %s",
             g_config.cc,
             g_config.gcc_flags,
             g_cflags,
             g_config.is_freestanding ? "-ffreestanding" : "",
             outfile,
             temp_source_file,
             g_parser_ctx->has_async ? "-lpthread" : "",
             g_link_flags);
#endif

    if (g_config.verbose)
    {
        printf("[CMD] %s\n", cmd);
    }

    int ret = system(cmd);
    if (ret != 0)
    {
        printf("C compilation failed.\n");
        if (!g_config.emit_c)
        {
            remove(temp_source_file);
        }
        return 1;
    }

    if (!g_config.emit_c)
    {
        // remove("out.c"); // Keep it for debugging for now or follow flag
        remove(temp_source_file);
    }

    if (g_config.mode_run)
    {
        char run_cmd[2048];

#if defined(_WIN32)
        // Windows: no "./". Use quotes so paths with spaces work.
        snprintf(run_cmd, sizeof(run_cmd), "\"%s\"", outfile);
#else
        // POSIX: execute from current directory
        snprintf(run_cmd, sizeof(run_cmd), "\"./%s\"", outfile);
#endif

        ret = system(run_cmd);

        remove(outfile);
        zptr_plugin_mgr_cleanup();
        zen_trigger_global();

#if defined(WIFEXITED) && defined(WEXITSTATUS)
        return WIFEXITED(ret) ? WEXITSTATUS(ret) : ret;
#else
        return ret;
#endif
    }

    zptr_plugin_mgr_cleanup();
    zen_trigger_global();
    return 0;
}
