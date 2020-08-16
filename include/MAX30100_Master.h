#ifndef MAX30100_MASTER_H_
#define MAX30100_MASTER_H_

#include <stdint.h>
#include <stdbool.h>
#include "MAX30100_Registers.h"
#include "MAX30100_Filters.h"

#define CURRENT_ADJUSTMENT_PERIOD_MS        500
#define DEFAULT_IR_LED_CURRENT              MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT_START               MAX30100_LED_CURR_27_1MA
#define DC_REMOVER_ALPHA                    0.97

#define TTY "/dev/ttyO4"
#define TTY_BAUDRATE 115200
int uart_fd;

// Escape Sequences for aesthetics
#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

IIRFilter_t butterBandpass;

LEDCurrent irLedCurrent = DEFAULT_IR_LED_CURRENT;
uint8_t redLedCurrentIndex = (uint8_t)RED_LED_CURRENT_START;

uint32_t tsFirstBeatDetected = 0;
uint32_t tsLastBeatDetected = 0;
uint32_t tsLastBiasCheck = 0;
uint32_t tsLastCurrentAdjustment = 0;

void pulseOxCheckCurrentBias();
void* fifoThread(void *args);
void* workerThread(void *args);

#endif /* MAX30100_MASTER_H_ */
