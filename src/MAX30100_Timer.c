#include <pthread.h>

#include "MAX30100_Timer.h"
#include "MAX30100_Log.h"

static long int stepCounter = 0;
static pthread_mutex_t timerLock = PTHREAD_MUTEX_INITIALIZER;
float timeStep_ms;

long int millis() {
    long int time;

    pthread_mutex_lock(&timerLock);
	time = stepCounter * timeStep_ms;
    pthread_mutex_unlock(&timerLock);
    return time;
}

void addTimeStep(int N) {
    pthread_mutex_lock(&timerLock);
    stepCounter += N;
    pthread_mutex_unlock(&timerLock);
}

void setTimeStep(SamplingRate fs) {
    switch (fs) {
        case MAX30100_SAMPRATE_1000HZ:
            max_debug("Current sampling rate (fs) is 1000 Hz");
            timeStep_ms = 1;
            break;
        case MAX30100_SAMPRATE_800HZ:
            max_debug("Current sampling rate (fs) is 800 Hz");
            timeStep_ms = 1.25;
            break;
        case MAX30100_SAMPRATE_600HZ:
            max_debug("Current sampling rate (fs) is 600 Hz");
            timeStep_ms = 1.6667;
            break;
        case MAX30100_SAMPRATE_400HZ:
            max_debug("Current sampling rate (fs) is 400 Hz");
            timeStep_ms = 2.5;
            break;
        case MAX30100_SAMPRATE_200HZ:
            max_debug("Current sampling rate (fs) is 200 Hz");
            timeStep_ms = 5;
            break;
        case MAX30100_SAMPRATE_167HZ:
            max_debug("Current sampling rate (fs) is 167 Hz");
            timeStep_ms = 5.9880;
            break;
        case MAX30100_SAMPRATE_100HZ:
            max_debug("Current sampling rate (fs) is 100 Hz");
            timeStep_ms = 10;
            break;
        case MAX30100_SAMPRATE_50HZ:
            max_debug("Current sampling rate (fs) is 50 Hz");
            timeStep_ms = 20;
            break;
        default:
            max_error("Fallback to default sampling rate (fs) 100 Hz");
            timeStep_ms = 10;
    }
}
