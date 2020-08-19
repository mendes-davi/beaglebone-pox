#ifndef _MAX30100_TIMER_H_
#define _MAX30100_TIMER_H_

#include "MAX30100_Registers.h"

long int millis();
void addTimeStep(int N);
void setTimeStep(SamplingRate fs);

#endif
