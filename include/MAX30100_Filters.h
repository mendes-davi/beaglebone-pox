#ifndef MAX30100_FILTERS_H_
#define MAX30100_FILTERS_H_

typedef struct {
    const float *a, *b; // filter coefs.
    float *px, *py; // previous inputs and outputs
    int N,M; // numerator and denominator order
} IIRFilter_t;

float IIRFilterStep(IIRFilter_t *H, float x);

// http://www.schwietering.com/jayduino/filtuino/
float v[3];

float butterworthStep(float x);
void setDCAlpha(float alpha_, char c);
float getDCW(char c);
float dcStepRed(float xRed);
float dcStepIr(float xIr);



#endif /* MAX30100_FILTERS_H_ */
