/*

	Example of use of the FFT libray to compute FFT for several signals over a range of frequencies.
        The exponent is calculated once before the excecution since it is a constant.
        This saves resources during the excecution of the sketch and reduces the compiled size.
        The sketch shows the time that the computing is taking.
        
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

#include "arduinoFFT.h"

/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 64;
const double sampling = 40;
const uint8_t amplitude = 4;
const double startFrequency = 2;
const double stopFrequency = 16.4;
const double step_size = 0.1;

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

/* Create FFT object */
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, sampling);

unsigned long startTime;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop()
{
  Serial.println("Frequency\tDetected\ttakes (ms)");
  Serial.println("=======================================\n");
  for(double frequency = startFrequency; frequency<=stopFrequency; frequency+=step_size)
  {
    /* Build raw data */
    double cycles = (((samples-1) * frequency) / sampling);
    for (uint16_t i = 0; i < samples; i++)
    {
      vReal[i] = int8_t((amplitude * (sin((i * (TWO_PI * cycles)) / samples))) / 2.0);
      vImag[i] = 0; //Reset the imaginary values vector for each new frequency
    }
    /*Serial.println("Data:");
    PrintVector(vReal, samples, SCL_TIME);*/
    startTime=millis();
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
    /*Serial.println("Weighed data:");
    PrintVector(vReal, samples, SCL_TIME);*/
    FFT.compute(FFTDirection::Forward); /* Compute FFT */
    /*Serial.println("Computed Real values:");
    PrintVector(vReal, samples, SCL_INDEX);
    Serial.println("Computed Imaginary values:");
    PrintVector(vImag, samples, SCL_INDEX);*/
    FFT.complexToMagnitude(); /* Compute magnitudes */
    /*Serial.println("Computed magnitudes:");
    PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);*/
    double x = FFT.majorPeak();
    Serial.print(frequency);
    Serial.print(": \t\t");
    Serial.print(x, 4);
    Serial.print("\t\t");
    Serial.print(millis()-startTime);
    Serial.println(" ms");
    // delay(2000); /* Repeat after delay */
  }
  while(1); /* Run Once */
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / sampling);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * sampling) / samples);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
