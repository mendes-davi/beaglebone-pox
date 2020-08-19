#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "MAX30100_Master.h"
#include "MAX30100_Log.h"
#include "MAX30100.h"
#include "wiringSerial.h"
#include "I2C.h"
#include "MAX30100_Timer.h"
#include "fdacoefs.h"

int main(int argc, char *argv[]) {
    pthread_t threadId[2];

    // Log
    if(max_init_log() == 0)
        max_info("Hello World!");
    else {
        printf("FAILED to start log!");
        exit(1);
    }

    // UART
    uart_fd = serialOpen(TTY, TTY_BAUDRATE);
    if (uart_fd == -2) {
        perror("UART: Wrong baudrate!");
        exit(1);
    }
    if (uart_fd == -1) {
        perror("UART: Failed to open tty!");
        exit(1);
    }


    // Initial Start of MAX30100
    // begin() uses the DEFAULT settings defined in MAX30100.h
	if(!begin()) {
        printf("Failed to initialize MAX30100");
        exit(1);
	}
    // Initial LED Current Parameters
	setMode(MAX30100_MODE_SPO2_HR);
    setLedsCurrent(irLedCurrent, (LEDCurrent) redLedCurrentIndex);

    // Initial Filtering Parameters
    setDCAlpha(DC_REMOVER_ALPHA, 'R');
    setDCAlpha(DC_REMOVER_ALPHA, 'I');
    // Butterworth Bandpass
    butterBandpass.M = M;
    butterBandpass.N = N;
    butterBandpass.a = den;
    butterBandpass.b = num;
    butterBandpass.px = malloc(M * sizeof(float));
    memset(butterBandpass.px, 0, M * sizeof(float));
    butterBandpass.py = malloc(N * sizeof(float));
    memset(butterBandpass.py, 0, N * sizeof(float));

    int err;
    err = pthread_create(&(threadId[0]), NULL, &fifoThread, NULL);
    if (err != 0) {
        printf("Can't create thread :[%s]\n", strerror(err));
        exit(1);
    }
    err = pthread_create(&(threadId[1]), NULL, &workerThread, NULL);
    if (err != 0) {
        printf("Can't create thread :[%s]\n", strerror(err));
        exit(1);
    }

    clear();
    while(millis() <= 10*1000)
    {
        /* gotoxy(0,0); */
        /* printf("\rTIME: %lu\n", millis()); */
    }

    shutdown();
    pthread_cancel(threadId[0]);
    pthread_cancel(threadId[1]);
    Close_device(I2C_DEV_1.fd_i2c);
    serialClose(uart_fd);
    return 0;
}

void* fifoThread(void *args) {
    while(true) {
        readFifoData();
    }
    return NULL;
}

void* workerThread(void *args) {
    FILE *fraw, *fdata;
    fraw = fopen("rawdata.txt", "w+");
    fdata = fopen("filtdata.txt", "w+");
    if (fraw == NULL || fdata == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    while(true) {
        uint16_t rawIRValue, rawRedValue;
        float irACValue, redACValue, filteredPulseValue;
        // Dequeue all available samples
        while(getRawValues(&rawIRValue, &rawRedValue)) {
            irACValue = dcStepIr(rawIRValue);
            redACValue = dcStepRed(rawRedValue);
            filteredPulseValue = IIRFilterStep(&butterBandpass, -rawIRValue);

            if(fprintf(fraw, "%u;\n", rawIRValue) < 0) {
                printf("Error while logging data!\n");
            }
            if(fprintf(fdata, "%.1f;\n", filteredPulseValue) < 0) {
                printf("Error while logging data!\n");
            }

            serialPrintf(uart_fd, "%f\n", filteredPulseValue);
        }

        // Balance R/IR LED Currents to achieve comparable DC baseline
        pulseOxCheckCurrentBias();
    }
    return NULL;
}

void pulseOxCheckCurrentBias() {
    // Follower that adjusts the red led current in order to have comparable DC baselines between
    // red and IR leds. The numbers are really magic: the less possible to avoid oscillations
    if (millis() - tsLastBiasCheck > CURRENT_ADJUSTMENT_PERIOD_MS) {
        bool changed = false;
        if (getDCW('I') - getDCW('R') > 70000 && redLedCurrentIndex < MAX30100_LED_CURR_50MA) {
            ++redLedCurrentIndex;
            changed = true;
        } else if (getDCW('R') - getDCW('I') > 70000 && redLedCurrentIndex > 0) {
            --redLedCurrentIndex;
            changed = true;
        }

        if (changed) {
            setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
            tsLastCurrentAdjustment = millis();
            //#TODO: Log the Current Change
        }
        tsLastBiasCheck = millis();
    }
}
