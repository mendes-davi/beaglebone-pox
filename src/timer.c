#include <pthread.h>

#include "timer.h"

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
            timeStep_ms = 1;
            break;
        case MAX30100_SAMPRATE_800HZ:
            timeStep_ms = 1.25;
            break;
        case MAX30100_SAMPRATE_600HZ:
            timeStep_ms = 1.6667;
            break;
        case MAX30100_SAMPRATE_400HZ:
            timeStep_ms = 2.5;
            break;
        case MAX30100_SAMPRATE_200HZ:
            timeStep_ms = 5;
            break;
        case MAX30100_SAMPRATE_167HZ:
            timeStep_ms = 5.9880;
            break;
        case MAX30100_SAMPRATE_100HZ:
            timeStep_ms = 10;
            break;
        case MAX30100_SAMPRATE_50HZ:
            timeStep_ms = 20;
            break;
        default:
            timeStep_ms = 10;
    }
}
