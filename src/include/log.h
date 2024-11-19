#ifndef LOG_H_
#define LOG_H_

FILE* open_log_file (const char* filename);

int log_printf (const char* message, ...);

int write_log_file (const char* reason_bro);

void close_log_file (void);

#endif
