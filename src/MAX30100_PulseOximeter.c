#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "MAX30100_PulseOximeter.h"
#include "MAX30100_SpO2Calculator.h"
#include "I2C.h"
#include "timer.h"
#include "MAX30100.h"
#include "MAX30100_BeatDetector.h"
/* #include "SSD1306_OLED.h" */

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

int main(void)
{
    unsigned int spo2Read, hrRead;
    uint32_t timeDisplay = 5000;


//    /* Initialize I2C bus and connect to the I2C Device */
//    if(init_i2c_dev2(SSD1306_OLED_ADDR) == 0)
//    {
//        printf("(Main)i2c-2: Bus Connected to SSD1306\r\n");
//    }
//    else
//    {
//        printf("(Main)i2c-2: OOPS! Something Went Wrong\r\n");
//        exit(1);
//    }
//    /* Run SDD1306 Initialization Sequence */
//    display_Init_seq();
//    /* Clear display */
//    clearDisplay();
//    setTextSize(2);
//    setTextColor(WHITE);
//    setCursor(10,5);
//    print_str("SpO2:");
//    setCursor(10,30);
//    print_str("HR:");
//    Display();

    // TODO Fiz POX begin routine
    /* Initialize I2C bus and connect to the I2C Device */
    if(pulseOxBegin(PULSEOXIMETER_DEBUGGINGMODE_NONE))
        printf("POX Connected!\r\n");
    else
        printf("POX Failed :(\r\n");
    //Setting Fs to 400 Hz and ADC to 14 bits
    /* shutdown(); */
    /* setHighresModeEnabled(false); */
    /* setLedsPulseWidth(MAX30100_SPC_PW_400US_14BITS); */
    /* setSamplingRate(MAX30100_SAMPRATE_400HZ); */
    /* resume(); */

    // TODO Fix File Logs
    /* Open .txt files to save POX samples. */
    fraw = fopen("rawdata.txt", "w+");
    fdata = fopen("filtdata.txt", "w+");
    if (fraw == NULL || fdata == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    clear();
    /* MAIN LOOP */
    // TODO Implement threads in main loop
    while(millis() < 25*1000)
    {
        update(); // Read data from MAX30100 FIFO
        pulseOxCheckSample(); // Filter sample and run beat detection
        pulseOxCheckCurrentBias(); // Check LEDS current

        spo2Read = (unsigned int) spO2CalcGetSpO2();
        hrRead = (unsigned int) beatDetectorGetRate();
        printf("SpO2: %u HR: %u \n", spo2Read, hrRead);
        printf("\rTIME: %lu\n", millis());
        gotoxy(5,5);

//        if (millis() >= timeDisplay)
//        {
//            shutdown();
//            timeDisplay +=3000;
//            clearDisplay();
//            setCursor(10,5);
//            print_str("SpO2:");
//            setCursor(10,30);
//            print_str("HR:");
//            setCursor(80,5);
//            if(spo2Read > 100 || spo2Read < 93)
//                print_str("##");
//            else
//                printNumber_UI(spo2Read,DEC);
//            setCursor(80,30);
//            printNumber_UI(hrRead,DEC);
//            Display();
//            resume();
//        }
    }
    fclose(fraw); // Closing files
    fclose(fdata);

//    clearDisplay();
//    setCursor(10,10);
//    print_str("END.");
//    Display();
    return 0;
}

bool pulseOxBegin(PulseOximeterDebuggingMode debuggingMode_)
{
	debuggingMode = debuggingMode_;

	bool ready = begin();

	if(!ready)
	{
		if(debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE)
			printf("Failed to initialize the HRM sensor");
		return false;
	}

	setMode(MAX30100_MODE_SPO2_HR);
    setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);

    setDCAlpha(DC_REMOVER_ALPHA, 'R');
    setDCAlpha(DC_REMOVER_ALPHA, 'I');

    state = PULSEOXIMETER_STATE_IDLE;

    return true;
}

void pulseOxCheckSample()
{
	uint16_t rawIRValue, rawRedValue;
	float irACValue, redACValue, filteredPulseValue;
	bool beatDetected;

	// Dequeue all available samples
	while(getRawValues(&rawIRValue, &rawRedValue))
	{
        if(fprintf(fraw, "%u;\n", rawIRValue) < 0) // Save IR value in  file
        {
            printf("Error while logging data!\n");
        }
        irACValue = dcStepIr(rawIRValue);
        redACValue = dcStepRed(rawRedValue);

        filteredPulseValue = butterworthStep(-irACValue);
        if(fprintf(fdata, "%f;\n", filteredPulseValue) < 0) // Save filtered value in file
        {
            printf("Error while logging data!\n");
        }
        beatDetected = beatDetectorAddSample(filteredPulseValue);

		if (beatDetectorGetRate() > 0) {
            state = PULSEOXIMETER_STATE_DETECTING;
            spO2CalcUpdate(irACValue, redACValue, beatDetected);
        } else if (state == PULSEOXIMETER_STATE_DETECTING) {
            state = PULSEOXIMETER_STATE_IDLE;
            spO2CalcReset();
        }

        switch (debuggingMode) {
            case PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES:
                printf("IR: %u RED: %u \n", rawIRValue, rawRedValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES:
                printf("IRac: %.3f REDac: %f.3\n", irACValue, redACValue);
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT:
                printf("R: %f TH: %f\nMillis: %lu\n", filteredPulseValue, beatDetectorGetCurrentThreshold(), millis());
                break;

            case PULSEOXIMETER_DEBUGGINGMODE_PULSEPLOTTER:
                printf("%.3f\n", filteredPulseValue);
                break;

            default:
                break;
        }

        if (beatDetected && onBeatDetected) {
            onBeatDetected();
        }
	}
}

void pulseOxCheckCurrentBias()
{
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

            if (debuggingMode != PULSEOXIMETER_DEBUGGINGMODE_NONE) {
                printf("Icurr: %u\n", redLedCurrentIndex);
            }

        }
        tsLastBiasCheck = millis();
    }

}

uint8_t pulseOxGetRedLedCurrentBias()
{
    return redLedCurrentIndex;
}

void pulseOxSetOnBeatDetectedCallback(void (*cb)())
{
    onBeatDetected = cb;
}

void pulseOxSetIRLedCurrent(LEDCurrent irLedNewCurrent)
{
    irLedCurrent = irLedNewCurrent;
    setLedsCurrent(irLedCurrent, (LEDCurrent)redLedCurrentIndex);
}
