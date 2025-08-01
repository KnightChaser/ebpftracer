// src/utils/logger.c
#define _GNU_SOURCE
#include "logger.h"
#include <stdio.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31;1m"
#define COLOR_YELLOW "\033[33;1m"
#define COLOR_CYAN "\033[36;1m"
#define COLOR_BOLD "\033[1m"
#define COLOR_GRAY "\033[90m"

static bool g_use_color = true;
static bool g_quiet = false;
static log_level_t g_min_level = LOG_INFO;

/**
 * Logs a message at the specified log level.
 *
 * @param level The log level. (log_level_t)
 */
void log_set_min_level(log_level_t level) { g_min_level = level; }

/**
 * Logs a message at the debug level.
 *
 * @param yes If true, enables colored output.
 */
void log_enable_color(bool yes) { g_use_color = yes; }

/**
 * Logs a message at the debug level.
 *
 * @param yes If true, suppresses all output.
 */
void log_set_quiet(bool yes) { g_quiet = yes; }

/**
 * Get the ANSI color code for the specified color.
 * If the color option is disabled (g_use_color is false),
 * returns an empty string, thus coloring will be disabled.
 *
 * @param c The color code as a string.
 */
static inline const char *get_color_ansi_code(const char *c) {
    return g_use_color ? c : "";
}

/**
 * Prints a formatted message to the standard output or error stream
 * based on the log level.
 *
 * @param level The log level (log_level_t).
 * @param tag The tag to prepend to the message.
 * @param fmt The format string for the message.
 * @param args The variable argument list for the format string.
 */
static void vprint_std(log_level_t level, const char *tag, const char *fmt,
                       va_list args) {
    if (g_quiet || level < g_min_level) {
        // If logging is not enabled or the level is below the minimum, return
        // early not to print anything.
        return;
    }

    FILE *out = (level >= LOG_WARN) ? stderr : stdout;
    fprintf(out, "%s[*]%s %s", get_color_ansi_code(COLOR_BOLD),
            get_color_ansi_code(COLOR_RESET), tag);
    if (tag[0]) {
        // If a tag is provided, print it followed by a space.
        fputc(' ', out);
    }

    vfprintf(out, fmt, args);
    fputc('\n', out);

    if (g_use_color) {
        // If color is enabled, reset the color at the end of the line.
        fputs(COLOR_RESET, out);
    }
}

/**
 * Logs a message at the debug level.
 *
 * @param fmt The format string for the message.
 * @param ... The variable arguments for the format string.
 * */
void log_debug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%sDBG:%s",
             get_color_ansi_code(COLOR_GRAY), get_color_ansi_code(COLOR_RESET));

    vprint_std(LOG_DEBUG, prefix, fmt, ap);

    va_end(ap);
}

/**
 * Logs a message at the info level.
 *
 * @param fmt The format string for the message.
 * @param ... The variable arguments for the format string.
 */
void log_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char prefix[64];
    snprintf(prefix, sizeof(prefix), "INF:");

    vprint_std(LOG_INFO, prefix, fmt, ap);

    va_end(ap);
}

/**
 * Logs a message at the warning level.
 *
 * @param fmt The format string for the message.
 * @param ... The variable arguments for the format string.
 */
void log_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%sWRN:%s",
             get_color_ansi_code(COLOR_YELLOW),
             get_color_ansi_code(COLOR_RESET));

    vprint_std(LOG_WARN, prefix, fmt, ap);

    va_end(ap);
}

/**
 * Logs a message at the error level.
 *
 * @param fmt The format string for the message.
 * @param ... The variable arguments for the format string.
 */
void log_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char prefix[64];
    snprintf(prefix, sizeof(prefix), "%sERR:%s", get_color_ansi_code(COLOR_RED),
             get_color_ansi_code(COLOR_RESET));

    vprint_std(LOG_ERR, prefix, fmt, ap);

    va_end(ap);
}

// Specialized syscall logging function

/**
 * Logs a syscall with its number, name, arguments, and return value.
 *
 * @param num_syscall The syscall number.
 * @param name The syscall name (can be NULL).
 * @param args_fmt The format string for the syscall arguments (can be NULL).
 * @param retval The return value of the syscall.
 */
void log_syscall(long num_syscall, const char *name, const char *args_fmt,
                 long retval) {
    if (g_quiet || LOG_SYSCALL < g_min_level) {
        return;
    }

    const char *c_cyan = get_color_ansi_code(COLOR_CYAN);
    const char *c_yellow = get_color_ansi_code(COLOR_YELLOW);
    const char *c_gray = get_color_ansi_code(COLOR_GRAY);
    const char *c_bold = get_color_ansi_code(COLOR_BOLD);
    const char *c_reset = get_color_ansi_code(COLOR_RESET);

    // Build argument string
    char argbuf[256] = "";
    if (args_fmt && *args_fmt)
        snprintf(argbuf, sizeof(argbuf), "(%s)", args_fmt);

    // Print:  [bold][cyan]NNN[reset]   [yellow]name[reset]  (args)  = 0x...
    printf("%s%s%03ld%s %-16s%s %s%s = %s%#lx%s\n", c_bold, c_cyan, num_syscall,
           c_reset,                                      // number
           c_yellow, (name ? name : "UNKNOWN"), c_reset, // name
           argbuf,                                       // arguments (args)
           c_gray, retval, c_reset);                     // return value (ret)

    // Bold is closed by the trailing reset
}

/**
 * Logs a return value with an optional tag.
 *
 * example: " = 0x1234 (tag)"
 *
 * @param retval The return value of the syscall.
 * @param tag An optional tag to append to the log message.
 */
void log_ret(long retval, const char *tag) {
    if (g_quiet || LOG_SYSCALL < g_min_level) {
        return;
    }

    printf("    %s=%s 0x%lx%s%s%s\n",
           get_color_ansi_code(COLOR_GRAY),   // +gray
           get_color_ansi_code(COLOR_RESET),  // -color
           retval,                            // return value
           *tag ? " " : "",                   // space if tag is present
           *tag ? tag : "",                   // tag
           get_color_ansi_code(COLOR_RESET)); // -color
}

/**
 * Logs a key-value pair with a formatted message.
 *
 * example: "    key=>value: ..."
 *
 * @param key The key to log.
 * @param fmt The format string for the value.
 * @param ... The variable arguments for the format string.
 */
void log_kv(const char *key, const char *fmt, ...) {
    if (g_quiet || LOG_SYSCALL < g_min_level) {
        return;
    }

    printf("    %s=>%s %s: ",
           get_color_ansi_code(COLOR_BOLD),  // +bold
           get_color_ansi_code(COLOR_RESET), // -color
           key);                             // key

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap); // Print the formatted message later
    va_end(ap);
    putchar('\n');
}

/**
 * Logs a hexdump of the provided buffer.
 *
 * @param indent The indentation level for the hexdump.
 * @param vbuf The buffer to log.
 * @param len The length of the buffer.
 */
void log_hexdump(unsigned indent, const void *vbuf, size_t len) {
    if (g_quiet || LOG_SYSCALL < g_min_level || len == 0) {
        return;
    }

    const unsigned char *buf = vbuf;
    const char *R = get_color_ansi_code(COLOR_RESET);
    const char *G = get_color_ansi_code(COLOR_GRAY);
    // const char *B = get_color_ansi_code(COLOR_BOLD); // yet unused :)

    for (size_t i = 0; i < len; i += 16) {
        // left margin + offset
        printf("%*s%s%04zx%s: ", indent, "", G, i, R);

        // hex bytes
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len)
                printf("%02x ", buf[i + j]);
            else
                printf("   "); // padding
            if (j == 7)
                putchar(' ');
        }

        // ASCII bar (print ASCII representation if possible)
        putchar('|');
        for (size_t j = 0; j < 16 && i + j < len; ++j) {
            unsigned char c = buf[i + j];
            putchar((c >= 0x20 && c <= 0x7e) ? c : '.');
        }
        puts("|");
    }
}
