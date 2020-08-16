#ifndef MAX30100_H
#define MAX30100_H

#include <stdbool.h>
#include "ringbuffer.h"
#include "MAX30100_Registers.h"
#include "I2C.h"
#include "timer.h"

#define DEFAULT_MODE                MAX30100_MODE_HRONLY
#define DEFAULT_SAMPLING_RATE       MAX30100_SAMPRATE_100HZ
#define DEFAULT_PULSE_WIDTH         MAX30100_SPC_PW_1600US_16BITS
#define DEFAULT_RED_LED_CURRENT     MAX30100_LED_CURR_50MA
#define DEFAULT_IR_LED_CURRENT      MAX30100_LED_CURR_50MA
#define EXPECTED_PART_ID            0x11
#define DEFAULT_BUFFER_SIZE_S       4

#define MAX30100_I2CADDR 0x57
extern I2C_DeviceT I2C_DEV_1;

ringBuffer_typedef(uint16_t, fifoBuffer);
fifoBuffer redBuffer, irBuffer;
fifoBuffer* redBuffer_ptr;
fifoBuffer* irBuffer_ptr;

bool begin();
bool isTemperatureReady();
bool getRawValues(uint16_t *ir, uint16_t *red);

uint8_t getPartId();
uint8_t readRegister(uint8_t address);
uint8_t retrieveTemperatureInteger();

float retrieveTemperature();

void writeRegister(uint8_t address, uint8_t data);
void setMode(Mode mode);
void setLedsPulseWidth(LEDPulseWidth ledPulseWidth);
void setSamplingRate(SamplingRate samplingRate);
void setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent);
void setHighresModeEnabled(bool enabled);
void resetFifo();
void resume();
void shutdown();
void startTemperatureSampling();
void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length);
void readFifoData();
void update();
int getRingBufferSize(SamplingRate samplingRate);

#endif
