/*

	FFT library
	Copyright (C) 2010 Didier Longueville
	Copyright (C) 2014 Enrique Condes
	Port to InifiniTime Copyright (C) 2023 Ceimour

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <cstdint>
#include <cmath>
#ifndef arduinoFFT_h /* Prevent loading library twice */
#define arduinoFFT_h
// #ifdef ARDUINO
// 	#if ARDUINO >= 100
// 		#include "Arduino.h"
// 	#else
// 		#include "WProgram.h" /* This is where the standard Arduino code lies */
// 	#endif
// #else
// 	#include <stdlib.h>
// 	#include <stdio.h>
// 	#ifdef __AVR__
// 		#include <avr/io.h>
// 		#include <avr/pgmspace.h>
// 	#endif
// 	#include <math.h>
// 	#include "defs.h"
// 	#include "types.h"
// #endif

#define FFT_LIB_REV 0x14
/* Custom constants */
#define FFT_FORWARD 0x01
#define FFT_REVERSE 0x00

/* Windowing type */
#define FFT_WIN_TYP_RECTANGLE 0x00 /* rectangle (Box car) */
#define FFT_WIN_TYP_HAMMING 0x01 /* hamming */
#define FFT_WIN_TYP_HANN 0x02 /* hann */
#define FFT_WIN_TYP_TRIANGLE 0x03 /* triangle (Bartlett) */
#define FFT_WIN_TYP_NUTTALL 0x04 /* nuttall */
#define FFT_WIN_TYP_BLACKMAN 0x05 /* blackman */
#define FFT_WIN_TYP_BLACKMAN_NUTTALL 0x06 /* blackman nuttall */
#define FFT_WIN_TYP_BLACKMAN_HARRIS 0x07 /* blackman harris*/
#define FFT_WIN_TYP_FLT_TOP 0x08 /* flat top */
#define FFT_WIN_TYP_WELCH 0x09 /* welch */
/*Mathematial constants*/
#define twoPi 6.28318531f
#define fourPi 12.56637061f
#define sixPi 18.84955593f

#ifdef __AVR__
	static const float _c1[]PROGMEM = {0.0000000000, 0.7071067812, 0.9238795325, 0.9807852804,
																0.9951847267, 0.9987954562, 0.9996988187, 0.9999247018,
																0.9999811753, 0.9999952938, 0.9999988235, 0.9999997059,
																0.9999999265, 0.9999999816, 0.9999999954, 0.9999999989,
																0.9999999997};
	static const float _c2[]PROGMEM = {1.0000000000, 0.7071067812, 0.3826834324, 0.1950903220,
																0.0980171403, 0.0490676743, 0.0245412285, 0.0122715383,
																0.0061358846, 0.0030679568, 0.0015339802, 0.0007669903,
																0.0003834952, 0.0001917476, 0.0000958738, 0.0000479369,
																0.0000239684};
#endif
class arduinoFFT {
public:
	/* Constructor */
	arduinoFFT(void);
	arduinoFFT(float *vReal, float *vImag, uint16_t samples, float samplingFrequency);
	/* Destructor */
	~arduinoFFT(void);
	/* Functions */
	uint8_t Revision(void);
	uint8_t Exponent(uint16_t value);

	void ComplexToMagnitude(float *vReal, float *vImag, uint16_t samples);
	void Compute(float *vReal, float *vImag, uint16_t samples, uint8_t dir);
	void Compute(float *vReal, float *vImag, uint16_t samples, uint8_t power, uint8_t dir);
	//void DCRemoval(float *vData, uint16_t samples);
	//float MajorPeak(float *vD, uint16_t samples, float samplingFrequency);
	//void MajorPeak(float *vD, uint16_t samples, float samplingFrequency, float *f, float *v);
	//void Windowing(float *vData, uint16_t samples, uint8_t windowType, uint8_t dir);

	//void ComplexToMagnitude();
	void Compute(uint8_t dir);
	//void DCRemoval();
	//float MajorPeak();
	//void MajorPeak(float *f, float *v);
	//void Windowing(uint8_t windowType, uint8_t dir);

	//float MajorPeakParabola();

private:
	/* Variables */
	uint16_t _samples;
	float _samplingFrequency;
	float *_vReal;
	float *_vImag;
	uint8_t _power;
	/* Functions */
	void Swap(float *x, float *y);
	//void Parabola(float x1, float y1, float x2, float y2, float x3, float y3, float *a, float *b, float *c);
};

#endif
