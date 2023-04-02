/*

	FFT library
	Copyright (C) 2010 Didier Longueville
	Copyright (C) 2014 Enrique Condes
	Copyright (C) 2020 Bim Overbohm (header-only, template, speed improvements)

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

#ifndef ArduinoFFT_h /* Prevent loading library twice */
#define ArduinoFFT_h
#ifdef ARDUINO
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h" /* This is where the standard Arduino code lies */
#endif
#else
#include <stdlib.h>
#include <stdio.h>
#ifdef __AVR__
#include <avr/io.h>
#include <avr/pgmspace.h>
#endif
#include <math.h>
#include "defs.h"
#include "types.h"
#endif

// Define this to use reciprocal multiplication for division and some more speedups that might decrease precision
//#define FFT_SPEED_OVER_PRECISION

// Define this to use a low-precision square root approximation instead of the regular sqrt() call
// This might only work for specific use cases, but is significantly faster. Only works for ArduinoFFT<float>.
//#define FFT_SQRT_APPROXIMATION

#ifdef FFT_SQRT_APPROXIMATION
	#include <type_traits>
#else
	#define sqrt_internal sqrtf
#endif

enum class FFTDirection
{
	Reverse,
	Forward
};

enum class FFTWindow
{
	Rectangle,		  // rectangle (Box car)
	Hamming,		  // hamming
	Hann,			  // hann
	Triangle,		  // triangle (Bartlett)
	Nuttall,		  // nuttall
	Blackman,		  //blackman
	Blackman_Nuttall, // blackman nuttall
	Blackman_Harris,  // blackman harris
	Flat_top,		  // flat top
	Welch			  // welch
};

template <typename T>
class ArduinoFFT
{
public:
	// Constructor
	ArduinoFFT(T *vReal, T *vImag, uint_fast16_t samples, T samplingFrequency, T *windowWeighingFactors = nullptr)
		: _vReal(vReal)
		, _vImag(vImag)
		, _samples(samples)
#ifdef FFT_SPEED_OVER_PRECISION
		, _oneOverSamples(1.0 / samples)
#endif
		, _samplingFrequency(samplingFrequency)
		, _windowWeighingFactors(windowWeighingFactors)
	{
		// Calculates the base 2 logarithm of sample count
		_power = 0;
		while (((samples >> _power) & 1) != 1)
		{
			_power++;
		}
	}

	// Destructor
	~ArduinoFFT()
	{
	}

	// Get library revision
	static uint8_t revision()
	{
		return 0x19;
	}

	// Replace the data array pointers
	void setArrays(T *vReal, T *vImag)
	{
		_vReal = vReal;
		_vImag = vImag;
	}

	// Computes in-place complex-to-complex FFT
	void compute(FFTDirection dir) const
	{
		// Reverse bits /
		uint_fast16_t j = 0;
		for (uint_fast16_t i = 0; i < (this->_samples - 1); i++)
		{
			if (i < j)
			{
				Swap(this->_vReal[i], this->_vReal[j]);
				if (dir == FFTDirection::Reverse)
				{
					Swap(this->_vImag[i], this->_vImag[j]);
				}
			}
			uint_fast16_t k = (this->_samples >> 1);
			while (k <= j)
			{
				j -= k;
				k >>= 1;
			}
			j += k;
		}
		// Compute the FFT
#ifdef __AVR__
		uint_fast8_t index = 0;
#endif
		T c1 = -1.0;
		T c2 = 0.0;
		uint_fast16_t l2 = 1;
		for (uint_fast8_t l = 0; (l < this->_power); l++)
		{
			uint_fast16_t l1 = l2;
			l2 <<= 1;
			T u1 = 1.0;
			T u2 = 0.0;
			for (j = 0; j < l1; j++)
			{
				for (uint_fast16_t i = j; i < this->_samples; i += l2)
				{
					uint_fast16_t i1 = i + l1;
					T t1 = u1 * this->_vReal[i1] - u2 * this->_vImag[i1];
					T t2 = u1 * this->_vImag[i1] + u2 * this->_vReal[i1];
					this->_vReal[i1] = this->_vReal[i] - t1;
					this->_vImag[i1] = this->_vImag[i] - t2;
					this->_vReal[i] += t1;
					this->_vImag[i] += t2;
				}
				T z = ((u1 * c1) - (u2 * c2));
				u2 = ((u1 * c2) + (u2 * c1));
				u1 = z;
			}
#ifdef __AVR__
			c2 = pgm_read_float_near(&(_c2[index]));
			c1 = pgm_read_float_near(&(_c1[index]));
			index++;
#else
			T cTemp = 0.5 * c1;
			c2 = sqrt_internal(0.5 - cTemp);
			c1 = sqrt_internal(0.5 + cTemp);
#endif
			c2 = dir == FFTDirection::Forward ? -c2 : c2;
		}
		// Scaling for reverse transform
		if (dir != FFTDirection::Forward)
		{
			for (uint_fast16_t i = 0; i < this->_samples; i++)
			{
#ifdef FFT_SPEED_OVER_PRECISION
				this->_vReal[i] *= _oneOverSamples;
				this->_vImag[i] *= _oneOverSamples;
#else
				this->_vReal[i] /= this->_samples;
				this->_vImag[i] /= this->_samples;
#endif
			}
		}
	}

	void complexToMagnitude() const
	{
		// vM is half the size of vReal and vImag
		for (uint_fast16_t i = 0; i < this->_samples; i++)
		{
			this->_vReal[i] = sqrt_internal(sq(this->_vReal[i]) + sq(this->_vImag[i]));
		}
	}

	void dcRemoval() const
	{
		// calculate the mean of vData
		T mean = 0;
		for (uint_fast16_t i = 1; i < ((this->_samples >> 1) + 1); i++)
		{
			mean += this->_vReal[i];
		}
		mean /= this->_samples;
		// Subtract the mean from vData
		for (uint_fast16_t i = 1; i < ((this->_samples >> 1) + 1); i++)
		{
			this->_vReal[i] -= mean;
		}
	}

	void windowing(FFTWindow windowType, FFTDirection dir, bool withCompensation = false)
	{
		// check if values are already pre-computed for the correct window type and compensation
		if (_windowWeighingFactors && _weighingFactorsComputed &&
			_weighingFactorsFFTWindow == windowType &&
			_weighingFactorsWithCompensation == withCompensation)
		{
			// yes. values are precomputed
			if (dir == FFTDirection::Forward)
			{
				for (uint_fast16_t i = 0; i < (this->_samples >> 1); i++)
				{
					this->_vReal[i] *= _windowWeighingFactors[i];
					this->_vReal[this->_samples - (i + 1)] *= _windowWeighingFactors[i];
				}
			}
			else
			{
				for (uint_fast16_t i = 0; i < (this->_samples >> 1); i++)
				{
#ifdef FFT_SPEED_OVER_PRECISION
					// on many architectures reciprocals and multiplying are much faster than division
					T oneOverFactor = 1.0 / _windowWeighingFactors[i];
					this->_vReal[i] *= oneOverFactor;
					this->_vReal[this->_samples - (i + 1)] *= oneOverFactor;
#else
					this->_vReal[i] /= _windowWeighingFactors[i];
					this->_vReal[this->_samples - (i + 1)] /= _windowWeighingFactors[i];
#endif
				}
			}
		}
		else
		{
			// no. values need to be pre-computed or applied
			T samplesMinusOne = (T(this->_samples) - 1.0);
			T compensationFactor = _WindowCompensationFactors[static_cast<uint_fast8_t>(windowType)];
			for (uint_fast16_t i = 0; i < (this->_samples >> 1); i++)
			{
				T indexMinusOne = T(i);
				T ratio = (indexMinusOne / samplesMinusOne);
				T weighingFactor = 1.0;
				// Compute and record weighting factor
				switch (windowType)
				{
				case FFTWindow::Rectangle: // rectangle (box car)
					weighingFactor = 1.0;
					break;
				case FFTWindow::Hamming: // hamming
					weighingFactor = 0.54 - (0.46 * cos(TWO_PI * ratio));
					break;
				case FFTWindow::Hann: // hann
					weighingFactor = 0.54 * (1.0 - cos(TWO_PI * ratio));
					break;
				case FFTWindow::Triangle: // triangle (Bartlett)
					weighingFactor = 1.0 - ((2.0 * abs(indexMinusOne - (samplesMinusOne / 2.0))) / samplesMinusOne);
					break;
				case FFTWindow::Nuttall: // nuttall
					weighingFactor = 0.355768 - (0.487396 * (cos(TWO_PI * ratio))) + (0.144232 * (cos(FOUR_PI * ratio))) - (0.012604 * (cos(SIX_PI * ratio)));
					break;
				case FFTWindow::Blackman: // blackman
					weighingFactor = 0.42323 - (0.49755 * (cos(TWO_PI * ratio))) + (0.07922 * (cos(FOUR_PI * ratio)));
					break;
				case FFTWindow::Blackman_Nuttall: // blackman nuttall
					weighingFactor = 0.3635819 - (0.4891775 * (cos(TWO_PI * ratio))) + (0.1365995 * (cos(FOUR_PI * ratio))) - (0.0106411 * (cos(SIX_PI * ratio)));
					break;
				case FFTWindow::Blackman_Harris: // blackman harris
					weighingFactor = 0.35875 - (0.48829 * (cos(TWO_PI * ratio))) + (0.14128 * (cos(FOUR_PI * ratio))) - (0.01168 * (cos(SIX_PI * ratio)));
					break;
				case FFTWindow::Flat_top: // flat top
					weighingFactor = 0.2810639 - (0.5208972 * cos(TWO_PI * ratio)) + (0.1980399 * cos(FOUR_PI * ratio));
					break;
				case FFTWindow::Welch: // welch
					weighingFactor = 1.0 - sq((indexMinusOne - samplesMinusOne / 2.0) / (samplesMinusOne / 2.0));
					break;
				}
				if (withCompensation)
				{
					weighingFactor *= compensationFactor;
				}
				if (_windowWeighingFactors)
				{
					_windowWeighingFactors[i] = weighingFactor;
				}
				if (dir == FFTDirection::Forward)
				{
					this->_vReal[i] *= weighingFactor;
					this->_vReal[this->_samples - (i + 1)] *= weighingFactor;
				}
				else
				{
#ifdef FFT_SPEED_OVER_PRECISION
					// on many architectures reciprocals and multiplying are much faster than division
					T oneOverFactor = 1.0 / weighingFactor;
					this->_vReal[i] *= oneOverFactor;
					this->_vReal[this->_samples - (i + 1)] *= oneOverFactor;
#else
					this->_vReal[i] /= weighingFactor;
					this->_vReal[this->_samples - (i + 1)] /= weighingFactor;
#endif
				}
			}
			// mark cached values as pre-computed
			_weighingFactorsFFTWindow = windowType;
			_weighingFactorsWithCompensation = withCompensation;
			_weighingFactorsComputed = true;
		}
	}

	T majorPeak() const
	{
		T maxY = 0;
		uint_fast16_t IndexOfMaxY = 0;
		//If sampling_frequency = 2 * max_frequency in signal,
		//value would be stored at position samples/2
		for (uint_fast16_t i = 1; i < ((this->_samples >> 1) + 1); i++)
		{
			if ((this->_vReal[i - 1] < this->_vReal[i]) && (this->_vReal[i] > this->_vReal[i + 1]))
			{
				if (this->_vReal[i] > maxY)
				{
					maxY = this->_vReal[i];
					IndexOfMaxY = i;
				}
			}
		}
		T delta = 0.5 * ((this->_vReal[IndexOfMaxY - 1] - this->_vReal[IndexOfMaxY + 1]) / (this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]));
		T interpolatedX = ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples - 1);
		if (IndexOfMaxY == (this->_samples >> 1))
		{
			//To improve calculation on edge values
			interpolatedX = ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples);
		}
		// returned value: interpolated frequency peak apex
		return interpolatedX;
	}

	void majorPeak(T &frequency, T &value) const
	{
		T maxY = 0;
		uint_fast16_t IndexOfMaxY = 0;
		//If sampling_frequency = 2 * max_frequency in signal,
		//value would be stored at position samples/2
		for (uint_fast16_t i = 1; i < ((this->_samples >> 1) + 1); i++)
		{
			if ((this->_vReal[i - 1] < this->_vReal[i]) && (this->_vReal[i] > this->_vReal[i + 1]))
			{
				if (this->_vReal[i] > maxY)
				{
					maxY = this->_vReal[i];
					IndexOfMaxY = i;
				}
			}
		}
		T delta = 0.5 * ((this->_vReal[IndexOfMaxY - 1] - this->_vReal[IndexOfMaxY + 1]) / (this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]));
		T interpolatedX = ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples - 1);
		if (IndexOfMaxY == (this->_samples >> 1))
		{
			//To improve calculation on edge values
			interpolatedX = ((IndexOfMaxY + delta) * this->_samplingFrequency) / (this->_samples);
		}
		// returned value: interpolated frequency peak apex
		frequency = interpolatedX;
		value = abs(this->_vReal[IndexOfMaxY - 1] - (2.0 * this->_vReal[IndexOfMaxY]) + this->_vReal[IndexOfMaxY + 1]);
	}

private:
#ifdef __AVR__
	static const float _c1[] PROGMEM;
	static const float _c2[] PROGMEM;
#endif
	static const T _WindowCompensationFactors[10];

	// Mathematial constants
#ifndef TWO_PI
	static constexpr T TWO_PI = 6.28318531; // might already be defined in Arduino.h
#endif
	static constexpr T FOUR_PI = 12.56637061;
	static constexpr T SIX_PI = 18.84955593;

	static inline void Swap(T &x, T &y)
	{
		T temp = x;
		x = y;
		y = temp;
	}

#ifdef FFT_SQRT_APPROXIMATION
	// Fast inverse square root aka "Quake 3 fast inverse square root", multiplied by x.
	// Uses one iteration of Halley's method for precision.
	// See: https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Iterative_methods_for_reciprocal_square_roots
	// And: https://github.com/HorstBaerbel/approx
	template <typename V = T>
	static inline V sqrt_internal(typename std::enable_if<std::is_same<V, float>::value, V>::type x)
	{
		union // get bits for float value
		{
			float x;
			int32_t i;
		} u;
		u.x = x;
		u.i = 0x5f375a86 - (u.i >> 1); // gives initial guess y0.
		float xu = x * u.x;
		float xu2 = xu * u.x;
		u.x = (0.125 * 3.0) * xu * (5.0 - xu2 * ((10.0 / 3.0) - xu2)); // Halley's method, repeating increases accuracy
		return u.x;
	}

	template <typename V = T>
	static inline V sqrt_internal(typename std::enable_if<std::is_same<V, double>::value, V>::type x)
	{
		// According to HosrtBaerbel, on the ESP32 the approximation is not faster, so we use the standard function
	#ifdef ESP32
		return sqrt(x);
	#else
		union // get bits for float value
		{
			double x;
			int64_t i;
		} u;
		u.x = x;
		u.i = 0x5fe6ec85e7de30da - (u.i >> 1); // gives initial guess y0.
		double xu = x * u.x;
		double xu2 = xu * u.x;
		u.x = (0.125 * 3.0) * xu * (5.0 - xu2 * ((10.0 / 3.0) - xu2)); // Halley's method, repeating increases accuracy
		return u.x;
	#endif
	}
#endif

	/* Variables */
	T *_vReal = nullptr;
	T *_vImag = nullptr;
	uint_fast16_t _samples = 0;
#ifdef FFT_SPEED_OVER_PRECISION
	T _oneOverSamples = 0.0;
#endif
	T _samplingFrequency = 0;
	T *_windowWeighingFactors = nullptr;
	FFTWindow _weighingFactorsFFTWindow;
	bool _weighingFactorsWithCompensation = false;
	bool _weighingFactorsComputed = false;
	uint_fast8_t _power = 0;
};

#ifdef __AVR__
template <typename T>
const float ArduinoFFT<T>::_c1[] PROGMEM = {
	0.0000000000, 0.7071067812, 0.9238795325, 0.9807852804,
	0.9951847267, 0.9987954562, 0.9996988187, 0.9999247018,
	0.9999811753, 0.9999952938, 0.9999988235, 0.9999997059,
	0.9999999265, 0.9999999816, 0.9999999954, 0.9999999989,
	0.9999999997};

template <typename T>
const float ArduinoFFT<T>::_c2[] PROGMEM = {
	1.0000000000, 0.7071067812, 0.3826834324, 0.1950903220,
	0.0980171403, 0.0490676743, 0.0245412285, 0.0122715383,
	0.0061358846, 0.0030679568, 0.0015339802, 0.0007669903,
	0.0003834952, 0.0001917476, 0.0000958738, 0.0000479369,
	0.0000239684};
#endif

template <typename T>
const T ArduinoFFT<T>::_WindowCompensationFactors[10] = {
	1.0000000000 * 2.0, // rectangle (Box car)
	1.8549343278 * 2.0, // hamming
	1.8554726898 * 2.0, // hann
	2.0039186079 * 2.0, // triangle (Bartlett)
	2.8163172034 * 2.0, // nuttall
	2.3673474360 * 2.0, // blackman
	2.7557840395 * 2.0, // blackman nuttall
	2.7929062517 * 2.0, // blackman harris
	3.5659039231 * 2.0, // flat top
	1.5029392863 * 2.0	// welch
};

#endif
