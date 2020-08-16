#ifndef _TIMER_H_
#define _TIMER_H_

#include "MAX30100_Registers.h"

long int millis();
void addTimeStep(int N);
void setTimeStep(SamplingRate fs);

#endif
