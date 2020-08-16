#include <string.h>

#include "MAX30100_Filters.h"

// http://sam-koblenski.blogspot.de/2015/11/everyday-dsp-for-programmers-dc-and.html
float alphaRed, alphaIr;
float dcwRed = 0, dcwIr = 0;

float butterworthStep(float x) //class II
{
	v[0] = v[1];
	v[1] = v[2];
	v[2] = (5.542717210280685182e-3 * x)
		 + (-0.80080264666570755150 * v[0])
		 + (1.77863177782458481424 * v[1]);
	return
		 (v[0] + v[2])
		+2 * v[1];
/*
	v[0] = v[1];
	v[1] = (2.452372752527856026e-1 * x)
		 + (0.50952544949442879485 * v[0]);
	return
		 (v[0] + v[1]);
*/
}

void setDCAlpha(float alpha_, char c)
{
	switch(c)
	{
		case 'R':
			alphaRed = alpha_;
			break;
		case 'I':
			alphaIr = alpha_;
			break;
		default:
			break;
	}
}

float getDCW(char c)
{
	switch(c)
	{
		case 'R':
			return dcwRed;
		case 'I':
			return dcwIr;
		default:
			return 0;
	}
}

float dcStepRed(float xRed)
{
	float olddcwRed = dcwRed;
	dcwRed = (float)xRed + alphaRed * dcwRed;

	return dcwRed - olddcwRed;
}

float dcStepIr(float xIr)
{
	float olddcwIr = dcwIr;
	dcwIr = (float)xIr + alphaIr * dcwIr;

	return dcwIr - olddcwIr;
}

float IIRFilterStep(IIRFilter_t *H, float x) {
    float y = 0; // output
    memmove(&H->px[0], &H->px[1], (H->M-1) * sizeof(float)); // shift previous samples
    H->px[H->M-1] = x; // append input

    // Filtering
    for(int l = 0; l < H->M; l++)
        y += H->b[l] * H->px[H->M-1-l];
    for(int i = 1; i < H->N; i++)
        y -= H->a[i] * H->py[H->N-1-i];

    H->py[H->N-1] = y; // append to previous outputs buffer
    memmove(&H->py[0], &H->py[1], (H->N-1) * sizeof(float)); // shift previous samples

    return (1/H->a[0]) * y; // In H, the 0th den. index is usually 1
}
