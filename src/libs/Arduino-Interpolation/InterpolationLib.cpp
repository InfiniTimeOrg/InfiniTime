/***************************************************
Copyright (c) 2019 Luis Llamas
(www.luisllamas.es)
Port to InfiniTime Copyright (C) 2023 Ceimour
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
 #include "InterpolationLib.h"

float Interpolation::Step(float xValues[], float yValues[], int numValues, float pointX, float threshold)
{
	// extremos
	if (pointX <= xValues[0]) return yValues[0];
	if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];

	auto i = 0;
	while (pointX >= xValues[i + 1]) i++;
		if (pointX == xValues[i + 1]) return yValues[i + 1]; // coincidencia exacta

	auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);  // punto relativo en el intervalo
	return t < threshold ? yValues[i] : yValues[i + 1];
}

float Interpolation::Linear(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
	if (trim)
	{
		if (pointX <= xValues[0]) return yValues[0];
		if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];
	}

	auto i = 0;
	float rst = 0;
	if (pointX <= xValues[0])
	{
		i = 0;
		auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
		rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
	}
	else if (pointX >= xValues[numValues - 1])
	{
		auto t = (pointX - xValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
		rst = yValues[numValues - 2] * (1 - t) + yValues[numValues - 1] * t;
	}
	else
	{
		while (pointX >= xValues[i + 1]) i++;
		auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
		rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
	}

	return rst;

}

float Interpolation::SmoothStep(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
	if (trim)
	{
		if (pointX <= xValues[0]) return yValues[0];
		if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];
	}

	auto i = 0;
	if (pointX <= xValues[0]) i = 0;
	else if (pointX >= xValues[numValues - 1]) i = numValues - 1;
	else while (pointX >= xValues[i + 1]) i++;
	if (pointX == xValues[i + 1]) return yValues[i + 1];

	auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
	t = t * t * (3 - 2 * t);
	return yValues[i] * (1 - t) + yValues[i + 1] * t;
}

float Interpolation::CatmullSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
	if (trim)
	{
		if (pointX <= xValues[0]) return yValues[0];
		if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];
	}

	auto i = 0;
	if (pointX <= xValues[0]) i = 0;
	else if (pointX >= xValues[numValues - 1]) i = numValues - 1;
	else while (pointX >= xValues[i + 1]) i++;
	if (pointX == xValues[i + 1]) return yValues[i + 1];

	auto t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
	auto t_2 = t * t;
	auto t_3 = t_2 * t;

	auto h00 = 2 * t_3 - 3 * t_2 + 1;
	auto h10 = t_3 - 2 * t_2 + t;
	auto h01 = 3 * t_2 - 2 * t_3;
	auto h11 = t_3 - t_2;

	auto x0 = xValues[i];
	auto x1 = xValues[i + 1];
	auto y0 = yValues[i];
	auto y1 = yValues[i + 1];

	float m0;
	float m1;
	if (i == 0)
	{
		m0 = (yValues[1] - yValues[0]) / (xValues[1] - xValues[0]);
		m1 = (yValues[2] - yValues[0]) / (xValues[2] - xValues[0]);
	}
	else if (i == numValues - 2)
	{
		m0 = (yValues[numValues - 1] - yValues[numValues - 3]) / (xValues[numValues - 1] - xValues[numValues - 3]);
		m1 = (yValues[numValues - 1] - yValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
	}
	else
	{
		m0 = catmullSlope(xValues, yValues, numValues, i);
		m1 = catmullSlope(xValues, yValues, numValues, i + 1);
	}

	auto rst = h00 * y0 + h01 * y1 + h10 * (x1 - x0) * m0 + h11 * (x1 - x0) * m1;
	return rst;
}

float Interpolation::catmullSlope(float x[], float y[], int n, int i)
{
	// Supress compiler warning for unused variable
	(void)n;
	
	if (x[i + 1] == x[i - 1]) return 0;
	return (y[i + 1] - y[i - 1]) / (x[i + 1] - x[i - 1]);
}

float Interpolation::ConstrainedSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
	if (trim)
	{
		if (pointX <= xValues[0]) return yValues[0];
		if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];
	}

	//auto i = 0;
	//while (pointX >= xValues[i + 1]) i++;
	//if (pointX == xValues[i + 1]) return yValues[i + 1];

	auto i = 0;
	if (pointX <= xValues[0]) i = 0;
	else if (pointX >= xValues[numValues - 1]) i = numValues - 1;
	else while (pointX >= xValues[i + 1]) i++;
	if (pointX == xValues[i + 1]) return yValues[i + 1];

	auto x0 = xValues[i + 1];
	auto x1 = xValues[i];
	auto y0 = yValues[i + 1];
	auto y1 = yValues[i];

	auto fd2i_xl1 = getLeftSecondDerivate(xValues, yValues, numValues - 1, i + 1);
	auto fd2i_x = getRightSecondDerivate(xValues, yValues, numValues - 1, i + 1);

	auto d = (fd2i_x - fd2i_xl1) / (6.0f * (x0 - x1));
	auto c = (x0 * fd2i_xl1 - x1 * fd2i_x) / 2.0f / (x0 - x1);
	auto b = (y0 - y1 - c * (x0 * x0 - x1 * x1) - d * (x0 * x0 * x0 - x1 * x1 * x1)) / (x0 - x1);
	auto a = y1 - b * x1 - c * x1 * x1 - d * x1 * x1 * x1;

	auto rst = a + pointX * (b + pointX * (c + pointX * d));
	return rst;
}


float Interpolation::getFirstDerivate(float x[], float y[], int n, int i)
{
	float fd1_x;

	if (i == 0)
	{
		fd1_x = 3.0f / 2.0f * (y[1] - y[0]) / (x[1] - x[0]);
		fd1_x -= getFirstDerivate(x, y, n, 1) / 2.0f;
	}
	else if (i == n)
	{
		fd1_x = 3.0f / 2.0f * (y[n] - y[n - 1]) / (x[n] - x[n - 1]);
		fd1_x -= getFirstDerivate(x, y, n, n - 1) / 2.0f;
	}
	else
	{
		if ((x[i + 1] - x[i]) / (y[i + 1] - y[i]) * (x[i] - x[i - 1]) / (y[i] - y[i - 1]) < 0)
		{
			fd1_x = 0;
		}
		else
		{
			fd1_x = 2.0f / ((x[i + 1] - x[i]) / (y[i + 1] - y[i]) + (x[i] - x[i - 1]) / (y[i] - y[i - 1]));
		}
	}
	return fd1_x;
}

float Interpolation::getLeftSecondDerivate(float x[], float y[], int n, int i)
{
	auto fdi_x = getFirstDerivate(x, y, n, i);
	auto fdi_xl1 = getFirstDerivate(x, y, n, i - 1);

	auto fd2l_x = -2.0f * (fdi_x + 2.0f * fdi_xl1) / (x[i] - x[i - 1]);
	fd2l_x += 6.0f * (y[i] - y[i - 1]) / (x[i] - x[i - 1]) / (x[i] - x[i - 1]);

	return fd2l_x;
}

float Interpolation::getRightSecondDerivate(float x[], float y[], int numValues, int i)
{
	auto fdi_x = getFirstDerivate(x, y, numValues, i);
	auto fdi_xl1 = getFirstDerivate(x, y, numValues, i - 1);

	auto fd2r_x = 2.0f * (2.0f * fdi_x + fdi_xl1) / (x[i] - x[i - 1]);
	fd2r_x -= 6.0f * (y[i] - y[i - 1]) / (x[i] - x[i - 1]) / (x[i] - x[i - 1]);

	return fd2r_x;
}