#include "os.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

void *z_dlopen(const char *path)
{
    return (void *)LoadLibraryA(path);
}

void *z_dlsym(void *handle, const char *symbol)
{
    return (void *)GetProcAddress((HMODULE)handle, symbol);
}

void z_dlclose(void *handle)
{
    if (handle)
    {
        FreeLibrary((HMODULE)handle);
    }
}

#else
#include <dlfcn.h>

void *z_dlopen(const char *path)
{
    return dlopen(path, RTLD_LAZY);
}

void *z_dlsym(void *handle, const char *symbol)
{
    return dlsym(handle, symbol);
}

void z_dlclose(void *handle)
{
    if (handle)
    {
        dlclose(handle);
    }
}
#endif
