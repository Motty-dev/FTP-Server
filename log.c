#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"

void log_message(const char *format, ...) {
    FILE *log_file = fopen("server.log", "a");
    if (log_file == NULL) {
        perror("Error opening server.log");
        return;
    }

    time_t now;
    struct tm *time_info;
    char time_str[64];

    time(&now);
    time_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]", time_info);

    fprintf(log_file, "%s ", time_str);

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fputc('\n', log_file);
    fclose(log_file);
}
