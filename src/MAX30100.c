#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "I2C.h"
#include "MAX30100.h"
#include "MAX30100_Timer.h"
#include "MAX30100_Log.h"

bool begin()
{
    if (init_i2c_dev1(MAX30100_I2CADDR) == -1) {
        max_trace("Failed to init I2C device");
        return false;
    }

    if(getPartId() != EXPECTED_PART_ID) {
        max_trace("Unexpected PART_ID for MAX30100");
    	return false;
    }

    setMode(DEFAULT_MODE);
    setLedsPulseWidth(DEFAULT_PULSE_WIDTH);
    setSamplingRate(DEFAULT_SAMPLING_RATE);
    setLedsCurrent(DEFAULT_IR_LED_CURRENT, DEFAULT_RED_LED_CURRENT);
    setHighresModeEnabled(true);
    resetFifo();

    int bufferSizeSamples = getRingBufferSize(DEFAULT_SAMPLING_RATE);
    bufferInit(redBuffer, bufferSizeSamples, uint16_t);
    bufferInit(irBuffer, bufferSizeSamples, uint16_t);
    max_debug("Initialized ringbuffer with size of %d samples", bufferSizeSamples);
    redBuffer_ptr = &redBuffer;
    irBuffer_ptr = &irBuffer;

    return true;
}

uint8_t getPartId()
{
	return readRegister(MAX30100_REG_PART_ID);
}

uint8_t readRegister(uint8_t address)
{
    unsigned char readData;
    i2c_read_register(I2C_DEV_1.fd_i2c, (unsigned char) address, &readData);
    return (uint8_t) readData;
}

void writeRegister(uint8_t address, uint8_t data)
{
    i2c_write_register(I2C_DEV_1.fd_i2c, (unsigned char) address, (unsigned char) data);
}

void burstRead(uint8_t baseAddress, uint8_t *buffer, uint8_t length)
{
    i2c_read_registers(I2C_DEV_1.fd_i2c, length,(unsigned char) baseAddress, (unsigned char *) buffer);
}

void setMode(Mode mode)
{
    writeRegister(MAX30100_REG_MODE_CONFIGURATION, mode);
}

void setLedsPulseWidth(LEDPulseWidth ledPulseWidth)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xfc) | ledPulseWidth);
}

void setSamplingRate(SamplingRate samplingRate)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    writeRegister(MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xe3) | (samplingRate << 2));
    // Setup Timer using fs
    setTimeStep(samplingRate);
}

void setLedsCurrent(LEDCurrent irLedCurrent, LEDCurrent redLedCurrent)
{
    writeRegister(MAX30100_REG_LED_CONFIGURATION, redLedCurrent << 4 | irLedCurrent);
    max_debug("Set LEDs Current to IR: %#x & R: %#x", irLedCurrent, redLedCurrent);
}

void setHighresModeEnabled(bool enabled)
{
    uint8_t previous = readRegister(MAX30100_REG_SPO2_CONFIGURATION);
    if (enabled) {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous | MAX30100_SPC_SPO2_HI_RES_EN);
    } else {
        writeRegister(MAX30100_REG_SPO2_CONFIGURATION, previous & ~MAX30100_SPC_SPO2_HI_RES_EN);
    }
    max_debug("High Resolution MAX30100 is set to %s", enabled ? "true" : "false");
}

void resetFifo()
{
    writeRegister(MAX30100_REG_FIFO_WRITE_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_READ_POINTER, 0);
    writeRegister(MAX30100_REG_FIFO_OVERFLOW_COUNTER, 0);
    max_debug("Reset MAX30100 fifo");
}

void resume()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig &= ~MAX30100_MC_SHDN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
    max_debug("Resumed MAX30100 Operation");
}

void shutdown()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_SHDN;
    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
    max_debug("MAX30100 is going down for power off");
}

float retrieveTemperature()
{
    int8_t tempInteger = readRegister(MAX30100_REG_TEMPERATURE_DATA_INT);
    float tempFrac = readRegister(MAX30100_REG_TEMPERATURE_DATA_FRAC);

    return tempFrac * 0.0625 + tempInteger;
}

uint8_t retrieveTemperatureInteger()
{
    return readRegister(MAX30100_REG_TEMPERATURE_DATA_INT);
}

bool isTemperatureReady()
{
    return !(readRegister(MAX30100_REG_MODE_CONFIGURATION) & MAX30100_MC_TEMP_EN);
}

void startTemperatureSampling()
{
    uint8_t modeConfig = readRegister(MAX30100_REG_MODE_CONFIGURATION);
    modeConfig |= MAX30100_MC_TEMP_EN;

    writeRegister(MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

void readFifoData()
{
	uint8_t buffer[MAX30100_FIFO_DEPTH*4];
    uint8_t toRead, i;
    uint16_t redWrite, irWrite;

    toRead = (readRegister(MAX30100_REG_FIFO_WRITE_POINTER) - readRegister(MAX30100_REG_FIFO_READ_POINTER)) & (MAX30100_FIFO_DEPTH-1);
    if(toRead)
    {
    	burstRead(MAX30100_REG_FIFO_DATA, buffer, 4 * toRead);
        addTimeStep((int) toRead);
    	for (i=0 ; i < toRead ; ++i)
    	{
    		irWrite = (uint16_t)((buffer[i*4] << 8) | buffer[i*4 + 1]);
    		redWrite = (uint16_t)((buffer[i*4 + 2] << 8) | buffer[i*4 + 3]);
    		bufferWrite(irBuffer_ptr,irWrite);
    		bufferWrite(redBuffer_ptr,redWrite);
    	}
    }
}

void update()
{
	readFifoData();
}

bool getRawValues(uint16_t *ir, uint16_t *red)
{
	if(!isBufferEmpty(irBuffer_ptr) && !isBufferEmpty(redBuffer_ptr))
	{
		bufferRead(irBuffer_ptr, *ir);
		bufferRead(redBuffer_ptr, *red);
		return true;
	}
	else
		return false;
}

int getRingBufferSize(SamplingRate samplingRate) {
    switch (samplingRate) {
        case MAX30100_SAMPRATE_1000HZ:
            return DEFAULT_BUFFER_SIZE_S * 1000;
        case MAX30100_SAMPRATE_800HZ:
            return DEFAULT_BUFFER_SIZE_S * 800;
        case MAX30100_SAMPRATE_600HZ:
            return DEFAULT_BUFFER_SIZE_S * 600;
        case MAX30100_SAMPRATE_400HZ:
            return DEFAULT_BUFFER_SIZE_S * 400;
        case MAX30100_SAMPRATE_200HZ:
            return DEFAULT_BUFFER_SIZE_S * 200;
        case MAX30100_SAMPRATE_167HZ:
            return DEFAULT_BUFFER_SIZE_S * 167;
        case MAX30100_SAMPRATE_100HZ:
            return DEFAULT_BUFFER_SIZE_S * 100;
        case MAX30100_SAMPRATE_50HZ:
            return DEFAULT_BUFFER_SIZE_S * 50;
        default:
           return 1024;
    }
}
