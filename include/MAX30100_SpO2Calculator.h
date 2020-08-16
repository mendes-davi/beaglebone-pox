#ifndef MAX30100_SPO2CALCULATOR_H_
#define MAX30100_SPO2CALCULATOR_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define CALCULATE_EVERY_N_BEATS 3

void spO2CalcUpdate(float irACValue, float redACValue, bool beatDetected);
void spO2CalcReset();
uint8_t spO2CalcGetSpO2();

// SaO2 Look-up Table
// http://www.ti.com/lit/an/slaa274b/slaa274b.pdf
const uint8_t spO2LUT[43] = {100,100,100,100,99,99,99,99,99,99,98,98,98,98,98,97,97,97,97,97,97,96,96,96,96,96,96,95,95,95,95,95,95,94,94,94,94,94,93,93,93,93,93};

float irACValueSqSum = 0;
float redACValueSqSum = 0;
uint8_t beatsDetectedNum = 0;
uint32_t samplesRecorded = 0;
uint8_t spO2 = 0;

uint8_t spO2CalcGetSpO2()
{
    return spO2;
}

void spO2CalcReset()
{
    samplesRecorded = 0;
    redACValueSqSum = 0;
    irACValueSqSum = 0;
    beatsDetectedNum = 0;
    spO2 = 0;
}

void spO2CalcUpdate(float irACValue, float redACValue, bool beatDetected)
{
    irACValueSqSum += irACValue * irACValue;
    redACValueSqSum += redACValue * redACValue;
    ++samplesRecorded;

    if (beatDetected) {
        ++beatsDetectedNum;
        if (beatsDetectedNum == CALCULATE_EVERY_N_BEATS) {
            float acSqRatio = 100.0 * log(redACValueSqSum/samplesRecorded) / log(irACValueSqSum/samplesRecorded);
            uint8_t index = 0;

            if (acSqRatio > 66) {
                index = (uint8_t)acSqRatio - 66;
            } else if (acSqRatio > 50) {
                index = (uint8_t)acSqRatio - 50;
            }
            spO2CalcReset();

            spO2 = spO2LUT[index];
        }
    }
}

#endif /* MAX30100_SPO2CALCULATOR_H_ */
