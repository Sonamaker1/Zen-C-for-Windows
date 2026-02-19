#include "cmd.h"
#include "../zprep.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void cmd_init(CmdBuilder *cmd)
{
    cmd->cap = 1024;
    cmd->len = 0;
    cmd->buf = xmalloc(cmd->cap);
    cmd->buf[0] = '\0';
}

static void ensure_cap(CmdBuilder *cmd, size_t needed)
{
    if (cmd->len + needed >= cmd->cap)
    {
        while (cmd->len + needed >= cmd->cap)
        {
            cmd->cap *= 2;
        }
        cmd->buf = xrealloc(cmd->buf, cmd->cap);
    }
}

void cmd_add(CmdBuilder *cmd, const char *str)
{
    if (!str || !str[0])
    {
        return;
    }

    size_t len = strlen(str);
    size_t needed = len + 1 + 1; // + space + null terminator

    ensure_cap(cmd, needed);

    if (cmd->len > 0 && cmd->buf[cmd->len - 1] != ' ')
    {
        strcat(cmd->buf, " ");
        cmd->len++;
    }

    strcat(cmd->buf, str);
    cmd->len += len;
}

void cmd_add_fmt(CmdBuilder *cmd, const char *fmt, ...)
{
    va_list args;

    // First pass to get size
    va_start(args, fmt);
    int size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (size < 0)
    {
        return;
    }

    size_t needed = size + 1 + 1; // + space + null
    ensure_cap(cmd, needed);

    if (cmd->len > 0 && cmd->buf[cmd->len - 1] != ' ')
    {
        strcat(cmd->buf, " ");
        cmd->len++;
    }

    va_start(args, fmt);
    vsnprintf(cmd->buf + cmd->len, cmd->cap - cmd->len, fmt, args);
    va_end(args);

    cmd->len += size;
}

void cmd_free(CmdBuilder *cmd)
{
    free(cmd->buf);
    cmd->buf = NULL;
    cmd->len = 0;
    cmd->cap = 0;
}

const char *cmd_to_string(CmdBuilder *cmd)
{
    return cmd->buf;
}
