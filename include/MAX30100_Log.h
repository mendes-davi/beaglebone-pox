#ifndef MAX30100_LOG_H
#define MAX30100_LOG_H

#include "log.h"

#define LOG_FILE "POX.log"
#define LOG_QUIET false
#define LOG_LEVEL LOG_TRACE
#define LOG_LEVEL_FILE LOG_TRACE

#define max_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define max_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define max_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define max_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define max_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define max_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

int max_init_log(void);
void max_log_lock(bool lock, void* udata);

#endif
