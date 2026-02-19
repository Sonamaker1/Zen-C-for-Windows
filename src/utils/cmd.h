#ifndef CMD_H
#define CMD_H

#include <stddef.h>

typedef struct
{
    char *buf;
    size_t len;
    size_t cap;
} CmdBuilder;

/**
 * @brief Initialize a new command builder
 * @param cmd The command builder to initialize
 */
void cmd_init(CmdBuilder *cmd);

/**
 * @brief Add a string to the command (appends space if needed)
 * @param cmd The command builder to add to
 * @param str The string to add
 */
void cmd_add(CmdBuilder *cmd, const char *str);

/**
 * @brief Add a format string to the command
 * @param cmd The command builder to add to
 * @param fmt The format string
 * @param ... The arguments to format
 */
void cmd_add_fmt(CmdBuilder *cmd, const char *fmt, ...);

/**
 * @brief Free the command builder
 * @param cmd The command builder to free
 */
void cmd_free(CmdBuilder *cmd);

/**
 * @brief Get the built command as a string (owned by the builder)
 * @param cmd The command builder to get the string from
 * @return The built command as a string
 */
const char *cmd_to_string(CmdBuilder *cmd);

#endif
