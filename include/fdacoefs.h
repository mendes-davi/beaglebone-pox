#ifndef FDACOEFS_H
#define FDACOEFS_H

// IIR Bandpass (1-40hz) 2nd Order Butterworth fs=400hz (via matlab filterDesigner)
const int N = 3;
const float num[3] = { 0.2402707970771, 0, -0.2402707970771 };
const int M = 3;
const float den[3] = { 1, -1.511722934732, 0.5194584058457 };

#endif
