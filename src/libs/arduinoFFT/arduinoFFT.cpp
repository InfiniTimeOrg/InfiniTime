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

#include "arduinoFFT.h"

arduinoFFT::arduinoFFT(void)
{ // Constructor
	//#warning("This method is deprecated and may be removed on future revisions.")
}

arduinoFFT::arduinoFFT(float *vReal, float *vImag, uint16_t samples, float samplingFrequency)
{// Constructor
	this->_vReal = vReal;
	this->_vImag = vImag;
	this->_samples = samples;
	this->_samplingFrequency = samplingFrequency;
	this->_power = Exponent(samples);
}

arduinoFFT::~arduinoFFT(void)
{
// Destructor
}

uint8_t arduinoFFT::Revision(void)
{
	return(FFT_LIB_REV);
}

void arduinoFFT::Compute(float *vReal, float *vImag, uint16_t samples, uint8_t dir)
{
	//#warning("This method is deprecated and may be removed on future revisions.")
	Compute(vReal, vImag, samples, Exponent(samples), dir);
}

void arduinoFFT::Compute(uint8_t dir)
{// Computes in-place complex-to-complex FFT /
	// Reverse bits /
	uint16_t j = 0;
	for (uint16_t i = 0; i < (this->_samples - 1); i++) {
		if (i < j) {
			Swap(&this->_vReal[i], &this->_vReal[j]);
			if(dir==FFT_REVERSE)
				Swap(&this->_vImag[i], &this->_vImag[j]);
		}
		uint16_t k = (this->_samples >> 1);
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	// Compute the FFT  /
#ifdef __AVR__
	uint8_t index = 0;
#endif
	float c1 = -1.0f;
	float c2 = 0.0f;
	uint16_t l2 = 1;
	for (uint8_t l = 0; (l < this->_power); l++) {
		uint16_t l1 = l2;
		l2 <<= 1;
		float u1 = 1.0f;
		float u2 = 0.0f;
		for (j = 0; j < l1; j++) {
			 for (uint16_t i = j; i < this->_samples; i += l2) {
					uint16_t i1 = i + l1;
					float t1 = u1 * this->_vReal[i1] - u2 * this->_vImag[i1];
					float t2 = u1 * this->_vImag[i1] + u2 * this->_vReal[i1];
					this->_vReal[i1] = this->_vReal[i] - t1;
					this->_vImag[i1] = this->_vImag[i] - t2;
					this->_vReal[i] += t1;
					this->_vImag[i] += t2;
			 }
			 float z = ((u1 * c1) - (u2 * c2));
			 u2 = ((u1 * c2) + (u2 * c1));
			 u1 = z;
		}
#ifdef __AVR__
		c2 = pgm_read_float_near(&(_c2[index]));
		c1 = pgm_read_float_near(&(_c1[index]));
		index++;
#else
		c2 = sqrtf((1.0f - c1) / 2.0f);
		c1 = sqrtf((1.0f + c1) / 2.0f);
#endif
		if (dir == FFT_FORWARD) {
			c2 = -c2;
		}
	}
	// Scaling for reverse transform /
	if (dir != FFT_FORWARD) {
		for (uint16_t i = 0; i < this->_samples; i++) {
			 this->_vReal[i] /= this->_samples;
			 this->_vImag[i] /= this->_samples;
		}
	}
}

void arduinoFFT::Compute(float *vReal, float *vImag, uint16_t samples, uint8_t power, uint8_t dir)
{	// Computes in-place complex-to-complex FFT
	// Reverse bits
	//#warning("This method is deprecated and may be removed on future revisions.")
	uint16_t j = 0;
	for (uint16_t i = 0; i < (samples - 1); i++) {
		if (i < j) {
			Swap(&vReal[i], &vReal[j]);
			if(dir==FFT_REVERSE)
				Swap(&vImag[i], &vImag[j]);
		}
		uint16_t k = (samples >> 1);
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
	// Compute the FFT
#ifdef __AVR__
	uint8_t index = 0;
#endif
	float c1 = -1.0f;
	float c2 = 0.0f;
	uint16_t l2 = 1;
	for (uint8_t l = 0; (l < power); l++) {
		uint16_t l1 = l2;
		l2 <<= 1;
		float u1 = 1.0f;
		float u2 = 0.0f;
		for (j = 0; j < l1; j++) {
			 for (uint16_t i = j; i < samples; i += l2) {
					uint16_t i1 = i + l1;
					float t1 = u1 * vReal[i1] - u2 * vImag[i1];
					float t2 = u1 * vImag[i1] + u2 * vReal[i1];
					vReal[i1] = vReal[i] - t1;
					vImag[i1] = vImag[i] - t2;
					vReal[i] += t1;
					vImag[i] += t2;
			 }
			 float z = ((u1 * c1) - (u2 * c2));
			 u2 = ((u1 * c2) + (u2 * c1));
			 u1 = z;
		}
#ifdef __AVR__
		c2 = pgm_read_float_near(&(_c2[index]));
		c1 = pgm_read_float_near(&(_c1[index]));
		index++;
#else
		c2 = sqrtf((1.0f - c1) / 2.0f);
		c1 = sqrtf((1.0f + c1) / 2.0f);
#endif
		if (dir == FFT_FORWARD) {
			c2 = -c2;
		}
	}
	// Scaling for reverse transform
	if (dir != FFT_FORWARD) {
		for (uint16_t i = 0; i < samples; i++) {
			 vReal[i] /= samples;
			 vImag[i] /= samples;
		}
	}
}

// void arduinoFFT::ComplexToMagnitude()
// { // vM is half the size of vReal and vImag
// 	for (uint16_t i = 0; i < this->_samples; i++) {
// 		this->_vReal[i] = sqrtf(powf(this->_vReal[i], 2) + powf(this->_vImag[i], 2));
// 	}
// }

void arduinoFFT::ComplexToMagnitude(float *vReal, float *vImag, uint16_t samples)
{	// vM is half the size of vReal and vImag
	//#warning("This method is deprecated and may be removed on future revisions.")
	for (uint16_t i = 0; i < samples; i++) {
		vReal[i] = sqrtf((vReal[i] * vReal[i]) + (vImag[i] * vImag[i]));
	}
}

// void arduinoFFT::DCRemoval()
// {
// 	// calculate the mean of vData
// 	float mean = 0;
// 	for (uint16_t i = 0; i < this->_samples; i++)
// 	{
// 		mean += this->_vReal[i];
// 	}
// 	mean /= this->_samples;
// 	// Subtract the mean from vData
// 	for (uint16_t i = 0; i < this->_samples; i++)
// 	{
// 		this->_vReal[i] -= mean;
// 	}
// }

// void arduinoFFT::DCRemoval(float *vData, uint16_t samples)
// {
// 	// calculate the mean of vData
// 	//#warning("This method is deprecated and may be removed on future revisions.")
// 	float mean = 0;
// 	for (uint16_t i = 0; i < samples; i++)
// 	{
// 		mean += vData[i];
// 	}
// 	mean /= samples;
// 	// Subtract the mean from vData
// 	for (uint16_t i = 0; i < samples; i++)
// 	{
// 		vData[i] -= mean;
// 	}
// }

// void arduinoFFT::Windowing(uint8_t windowType, uint8_t dir)
// {// Weighing factors are computed once before multiple use of FFT
// // The weighing function is symmetric; half the weighs are recorded
// 	float samplesMinusOne = (float(this->_samples) - 1.0f);
// 	for (uint16_t i = 0; i < (this->_samples >> 1); i++) {
// 		float indexMinusOne = float(i);
// 		float ratio = (indexMinusOne / samplesMinusOne);
// 		float weighingFactor = 1.0f;
// 		// Compute and record weighting factor
// 		switch (windowType) {
// 		case FFT_WIN_TYP_RECTANGLE: // rectangle (box car)
// 			weighingFactor = 1.0f;
// 			break;
// 		case FFT_WIN_TYP_HAMMING: // hamming
// 			weighingFactor = 0.54f - (0.46f * cosf(twoPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_HANN: // hann
// 			weighingFactor = 0.54f * (1.0f - cosf(twoPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_TRIANGLE: // triangle (Bartlett)
// 			#if defined(ESP8266) || defined(ESP32)
// 			weighingFactor = 1.0 - ((2.0 * fabs(indexMinusOne - (samplesMinusOne / 2.0))) / samplesMinusOne);
// 			#else
// 			weighingFactor = 1.0f - ((2.0f * abs(indexMinusOne - (samplesMinusOne / 2.0f))) / samplesMinusOne);
// 			#endif
// 			break;
// 		case FFT_WIN_TYP_NUTTALL: // nuttall
// 			weighingFactor = 0.355768f - (0.487396f * (cosf(twoPi * ratio))) + (0.144232f * (cosf(fourPi * ratio))) - (0.012604f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN: // blackman
// 			weighingFactor = 0.42323f - (0.49755f * (cosf(twoPi * ratio))) + (0.07922f * (cosf(fourPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN_NUTTALL: // blackman nuttall
// 			weighingFactor = 0.3635819f - (0.4891775f * (cosf(twoPi * ratio))) + (0.1365995f * (cosf(fourPi * ratio))) - (0.0106411f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN_HARRIS: // blackman harris
// 			weighingFactor = 0.35875f - (0.48829f * (cosf(twoPi * ratio))) + (0.14128f * (cosf(fourPi * ratio))) - (0.01168f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_FLT_TOP: // flat top
// 			weighingFactor = 0.2810639f - (0.5208972f * cosf(twoPi * ratio)) + (0.1980399f * cosf(fourPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_WELCH: // welch
// 			weighingFactor = 1.0f - powf((indexMinusOne - samplesMinusOne / 2.0) / (samplesMinusOne / 2.0), 2);
// 			break;
// 		}
// 		if (dir == FFT_FORWARD) {
// 			this->_vReal[i] *= weighingFactor;
// 			this->_vReal[this->_samples - (i + 1)] *= weighingFactor;
// 		}
// 		else {
// 			this->_vReal[i] /= weighingFactor;
// 			this->_vReal[this->_samples - (i + 1)] /= weighingFactor;
// 		}
// 	}
// }


// void arduinoFFT::Windowing(float *vData, uint16_t samples, uint8_t windowType, uint8_t dir)
// {// Weighing factors are computed once before multiple use of FFT
// // The weighing function is symetric; half the weighs are recorded
// 	//#warning("This method is deprecated and may be removed on future revisions.")
// 	float samplesMinusOne = (float(samples) - 1.0f);
// 	for (uint16_t i = 0; i < (samples >> 1); i++) {
// 		float indexMinusOne = float(i);
// 		float ratio = (indexMinusOne / samplesMinusOne);
// 		float weighingFactor = 1.0f;
// 		// Compute and record weighting factor
// 		switch (windowType) {
// 		case FFT_WIN_TYP_RECTANGLE: // rectangle (box car)
// 			weighingFactor = 1.0f;
// 			break;
// 		case FFT_WIN_TYP_HAMMING: // hamming
// 			weighingFactor = 0.54f - (0.46f * cosf(twoPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_HANN: // hann
// 			weighingFactor = 0.54f * (1.0f - cosf(twoPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_TRIANGLE: // triangle (Bartlett)
// 			#if defined(ESP8266) || defined(ESP32)
// 			weighingFactor = 1.0 - ((2.0 * fabs(indexMinusOne - (samplesMinusOne / 2.0))) / samplesMinusOne);
// 			#else
// 			weighingFactor = 1.0f - ((2.0f * abs(indexMinusOne - (samplesMinusOne / 2.0f))) / samplesMinusOne);
// 			#endif
// 			break;
// 		case FFT_WIN_TYP_NUTTALL: // nuttall
// 			weighingFactor = 0.355768f - (0.487396f * (cosf(twoPi * ratio))) + (0.144232f * (cosf(fourPi * ratio))) - (0.012604f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN: // blackman
// 			weighingFactor = 0.42323f - (0.49755f * (cosf(twoPi * ratio))) + (0.07922f * (cosf(fourPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN_NUTTALL: // blackman nuttall
// 			weighingFactor = 0.3635819f - (0.4891775f * (cosf(twoPi * ratio))) + (0.1365995f * (cosf(fourPi * ratio))) - (0.0106411f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_BLACKMAN_HARRIS: // blackman harris
// 			weighingFactor = 0.35875f - (0.48829f * (cosf(twoPi * ratio))) + (0.14128f * (cosf(fourPi * ratio))) - (0.01168f * (cosf(sixPi * ratio)));
// 			break;
// 		case FFT_WIN_TYP_FLT_TOP: // flat top
// 			weighingFactor = 0.2810639f - (0.5208972f * cosf(twoPi * ratio)) + (0.1980399f * cosf(fourPi * ratio));
// 			break;
// 		case FFT_WIN_TYP_WELCH: // welch
// 			weighingFactor = 1.0f - powf((indexMinusOne - samplesMinusOne / 2.0f) / (samplesMinusOne / 2.0f), 2);
// 			break;
// 		}
// 		if (dir == FFT_FORWARD) {
// 			vData[i] *= weighingFactor;
// 			vData[samples - (i + 1)] *= weighingFactor;
// 		}
// 		else {
// 			vData[i] /= weighingFactor;
// 			vData[samples - (i + 1)] /= weighingFactor;
// 		}
// 	}
// }

// float arduinoFFT::MajorPeak()
// {
// 	float maxY = 0;
// 	uint16_t IndexOfMaxY = 0;
// 	//If sampling_frequency = 2 * max_frequency in signal,
// 	//value would be stored at position samples/2
// 	for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++) {
// 		if ((this->_vReal[i-1] < this->_vReal[i]) && (this->_vReal[i] > this->_vReal[i+1])) {
// 			if (this->_vReal[i] > maxY) {
// 				maxY = this->_vReal[i];
// 				IndexOfMaxY = i;
// 			}
// 		}
// 	}
// 	float delta = 0.5f * ((this->_vReal[IndexOfMaxY-1] - this->_vReal[IndexOfMaxY+1]) / (this->_vReal[IndexOfMaxY-1] - (2.0f * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY+1]));
// 	float interpolatedX = ((IndexOfMaxY + delta)  * this->_samplingFrequency) / (this->_samples-1);
// 	if(IndexOfMaxY==(this->_samples >> 1)) //To improve calculation on edge values
// 		interpolatedX = ((IndexOfMaxY + delta)  * this->_samplingFrequency) / (this->_samples);
// 	// returned value: interpolated frequency peak apex
// 	return(interpolatedX);
// }

// void arduinoFFT::MajorPeak(float *f, float *v)
// {
// 	float maxY = 0;
// 	uint16_t IndexOfMaxY = 0;
// 	//If sampling_frequency = 2 * max_frequency in signal,
// 	//value would be stored at position samples/2
// 	for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++) {
// 		if ((this->_vReal[i - 1] < this->_vReal[i]) && (this->_vReal[i] > this->_vReal[i + 1])) {
// 			if (this->_vReal[i] > maxY) {
// 				maxY = this->_vReal[i];
// 				IndexOfMaxY = i;
// 			}
// 		}
// 	}
// 	float delta = 0.5f * ((this->_vReal[IndexOfMaxY - 1] - this->_vReal[IndexOfMaxY + 1]) / (this->_vReal[IndexOfMaxY - 1] - (2.0f * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]));
// 	float interpolatedX = ((IndexOfMaxY + delta)  * this->_samplingFrequency) / (this->_samples - 1);
// 	if (IndexOfMaxY == (this->_samples >> 1)) //To improve calculation on edge values
// 		interpolatedX = ((IndexOfMaxY + delta)  * this->_samplingFrequency) / (this->_samples);
// 	// returned value: interpolated frequency peak apex
// 	*f = interpolatedX;
// 	#if defined(ESP8266) || defined(ESP32)
// 	*v = fabs(this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]);
// 	#else
// 	*v = abs(this->_vReal[IndexOfMaxY - 1] - (2.0f * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]);
// 	#endif
// }

// float arduinoFFT::MajorPeak(float *vD, uint16_t samples, float samplingFrequency)
// {
// 	//#warning("This method is deprecated and may be removed on future revisions.")
// 	float maxY = 0;
// 	uint16_t IndexOfMaxY = 0;
// 	//If sampling_frequency = 2 * max_frequency in signal,
// 	//value would be stored at position samples/2
// 	for (uint16_t i = 1; i < ((samples >> 1) + 1); i++) {
// 		if ((vD[i-1] < vD[i]) && (vD[i] > vD[i+1])) {
// 			if (vD[i] > maxY) {
// 				maxY = vD[i];
// 				IndexOfMaxY = i;
// 			}
// 		}
// 	}
// 	float delta = 0.5f * ((vD[IndexOfMaxY-1] - vD[IndexOfMaxY+1]) / (vD[IndexOfMaxY-1] - (2.0f * vD[IndexOfMaxY]) + vD[IndexOfMaxY+1]));
// 	float interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples-1);
// 	if(IndexOfMaxY==(samples >> 1)) //To improve calculation on edge values
// 		interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples);
// 	// returned value: interpolated frequency peak apex
// 	return(interpolatedX);
// }

// void arduinoFFT::MajorPeak(float *vD, uint16_t samples, float samplingFrequency, float *f, float *v)
// {
// 	//#warning("This method is deprecated and may be removed on future revisions.")
// 	float maxY = 0;
// 	uint16_t IndexOfMaxY = 0;
// 	//If sampling_frequency = 2 * max_frequency in signal,
// 	//value would be stored at position samples/2
// 	for (uint16_t i = 1; i < ((samples >> 1) + 1); i++) {
// 		if ((vD[i - 1] < vD[i]) && (vD[i] > vD[i + 1])) {
// 			if (vD[i] > maxY) {
// 				maxY = vD[i];
// 				IndexOfMaxY = i;
// 			}
// 		}
// 	}
// 	float delta = 0.5f * ((vD[IndexOfMaxY - 1] - vD[IndexOfMaxY + 1]) / (vD[IndexOfMaxY - 1] - (2.0f * vD[IndexOfMaxY]) + vD[IndexOfMaxY + 1]));
// 	float interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples - 1);
// 	//float popo =
// 	if (IndexOfMaxY == (samples >> 1)) //To improve calculation on edge values
// 		interpolatedX = ((IndexOfMaxY + delta)  * samplingFrequency) / (samples);
// 	// returned value: interpolated frequency peak apex
// 	*f = interpolatedX;
// 	#if defined(ESP8266) || defined(ESP32)
// 	*v = fabs(vD[IndexOfMaxY - 1] - (2.0 * vD[IndexOfMaxY]) + vD[IndexOfMaxY + 1]);
// 	#else
// 	*v = abs(vD[IndexOfMaxY - 1] - (2.0f * vD[IndexOfMaxY]) + vD[IndexOfMaxY + 1]);
// 	#endif
// }

// float arduinoFFT::MajorPeakParabola()
// {
// 	float maxY = 0;
// 	uint16_t IndexOfMaxY = 0;
// 	//If sampling_frequency = 2 * max_frequency in signal,
// 	//value would be stored at position samples/2
// 	for (uint16_t i = 1; i < ((this->_samples >> 1) + 1); i++)
// 	{
// 		if ((this->_vReal[i-1] < this->_vReal[i]) && (this->_vReal[i] > this->_vReal[i+1]))
// 		{
// 			if (this->_vReal[i] > maxY)
// 			{
// 				maxY = this->_vReal[i];
// 				IndexOfMaxY = i;
// 			}
// 		}
// 	}

// 	float freq = 0;
// 	if( IndexOfMaxY>0 )
// 	{
// 		// Assume the three points to be on a parabola
// 		float a,b,c;
// 		Parabola(IndexOfMaxY-1, this->_vReal[IndexOfMaxY-1], IndexOfMaxY, this->_vReal[IndexOfMaxY], IndexOfMaxY+1, this->_vReal[IndexOfMaxY+1], &a, &b, &c);

// 		// Peak is at the middle of the parabola
// 		float x = -b/(2*a);

// 		// And magnitude is at the extrema of the parabola if you want It...
// 		// float y = a*x*x+b*x+c;  

// 		// Convert to frequency
// 		freq = (x  * this->_samplingFrequency) / (this->_samples);
// 	}

// 	return freq;
// }

// void arduinoFFT::Parabola(float x1, float y1, float x2, float y2, float x3, float y3, float *a, float *b, float *c)
// {
// 	float reversed_denom = 1/((x1 - x2) * (x1 - x3) * (x2 - x3));

// 	*a = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) * reversed_denom;
// 	*b = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) * reversed_denom;
// 	*c = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) *reversed_denom;
// }

uint8_t arduinoFFT::Exponent(uint16_t value)
{
	//#warning("This method may not be accessible on future revisions.")
	// Calculates the base 2 logarithm of a value
	uint8_t result = 0;
	while (((value >> result) & 1) != 1) result++;
	return(result);
}

// Private functions

void arduinoFFT::Swap(float *x, float *y)
{
	float temp = *x;
	*x = *y;
	*y = temp;
}
