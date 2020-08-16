#include <pthread.h>

#include "MAX30100_Log.h"

static pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;

int max_init_log(void) {
    FILE *fp_log;

    if((fp_log = fopen(LOG_FILE, "w")) == NULL)
        return -1;
    if(log_add_fp(fp_log, LOG_LEVEL_FILE) == -1)
        return -1;

    log_set_level(LOG_LEVEL);
    log_set_quiet(LOG_QUIET);
    log_set_lock(max_log_lock, &logLock);

    return 0;
}

void max_log_lock(bool lock, void* udata) {
  if (lock)
    pthread_mutex_lock(&logLock);
  else
    pthread_mutex_unlock(&logLock);
}
