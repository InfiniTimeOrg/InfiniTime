/***************************************************
Copyright (c) 2019 Luis Llamas
(www.luisllamas.es)
Port to InfiniTime Copyright (C) 2023 Ceimour
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
 #ifndef _INTERPOLATIONLIB_h
#define _INTERPOLATIONLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include <cstddef>
#endif


template<size_t n>
struct Range
{
	float list[n];

	Range()
	{
		for (size_t m = 0; m != n; ++m)
		{
			list[m] = m + 1;
		}
	}

	Range(float min, float max)
	{
		for (size_t m = 0; m < n; ++m)
		{
			list[m] = min + (max - min) / (n - 1) * m;
		}
	}

	float& operator[](size_t  index)
	{
		return list[index];
	}

	float* ToArray()
	{
		return list;
	}

	static float* Generate(float min, float max)
	{
		Range<10> range(min, max);
		return range.ToArray();
	}
};



class Interpolation
{
public:
	template <typename T>
	static T Map(T x, T in_min, T in_max, T out_min, T out_max);

	static float Step(float yValues[], int numValues, float pointX, float threshold = 1);
	static float Step(float minX, float maxX, float yValues[], int numValues, float pointX, float threshold = 1);
	static float Step(float xValues[], float yValues[], int numValues, float pointX, float threshold = 1);

	static float Linear(float yValues[], int numValues, float pointX, bool trim = true);
	static float Linear(float minX, float maxX, float yValues[], int numValues, float pointX, bool trim = true);
	static float Linear(float xValues[], float yValues[], int numValues, float pointX, bool trim = true);

	static float SmoothStep(float xValues[], float yValues[], int numValues, float pointX, bool trim = true);
	static float CatmullSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim = true);
	static float ConstrainedSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim = true);

private:
	static float catmullSlope(float x[], float y[], int n, int i);
	static float getFirstDerivate(float x[], float y[], int n, int i);
	static float getLeftSecondDerivate(float x[], float y[], int n, int i);
	static float getRightSecondDerivate(float x[], float y[], int n, int i);

};


// Esto esta aqui porque Arduino la lia con los Templates
template <typename T>
T Interpolation::Map(T x, T in_min, T in_max, T out_min, T out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif