arduinoFFT
==========

# Fast Fourier Transform for Arduino

This is a fork from https://code.google.com/p/makefurt/ which has been abandoned since 2011.
~~This is a C++ library for Arduino for computing FFT.~~ Now it works both on Arduino and C projects. This is version 2.0 of the library, which has a different [API](#api). See here [how to migrate from 1.x to 2.x](#migrating-from-1x-to-2x).
Tested on Arduino 1.6.11 and 1.8.10.

## Installation on Arduino

Use the Arduino Library Manager to install and keep it updated. Just look for arduinoFFT. Only for Arduino 1.5+

## Manual installation on Arduino

To install this library, just place this entire folder as a subfolder in your Arduino installation. When installed, this library should look like:

`Arduino\libraries\arduinoFTT` (this library's folder)  
`Arduino\libraries\arduinoFTT\src\arduinoFTT.h` (the library header file. include this in your project)  
`Arduino\libraries\arduinoFTT\keywords.txt` (the syntax coloring file)  
`Arduino\libraries\arduinoFTT\Examples` (the examples in the "open" menu)  
`Arduino\libraries\arduinoFTT\LICENSE` (GPL license file)  
`Arduino\libraries\arduinoFTT\README.md` (this file)

## Building on Arduino

After this library is installed, you just have to start the Arduino application.
You may see a few warning messages as it's built.  
To use this library in a sketch, go to the Sketch | Import Library menu and
select arduinoFTT.  This will add a corresponding line to the top of your sketch:

`#include <arduinoFTT.h>`

## API

* ```ArduinoFFT(T *vReal, T *vImag, uint_fast16_t samples, T samplingFrequency, T * weighingFactors = nullptr);```  
Constructor.
The type `T` can be `float` or `double`. `vReal` and `vImag` are pointers to arrays of real and imaginary data and have to be allocated outside of ArduinoFFT. `samples` is the number of samples in `vReal` and `vImag` and `weighingFactors` (if specified). `samplingFrequency` is the sample frequency of the data. `weighingFactors` can optionally be specified to cache weighing factors for the windowing function. This speeds up repeated calls to **windowing()** significantly. You can deallocate `vReal` and `vImag` after you are done using the library, or only use specific library functions that only need one of those arrays.

```C++
const uint32_t nrOfSamples = 1024;
auto real = new float[nrOfSamples];
auto imag = new float[nrOfSamples];
auto fft = ArduinoFFT<float>(real, imag, nrOfSamples, 10000);
// ... fill real + imag and use it ...
fft.compute();
fft.complexToMagnitude();
delete [] imag;
// ... continue using real and only functions that use real ...
auto peak = fft.majorPeak();
```
* ```~ArduinoFFT()```   
Destructor.
* ```void complexToMagnitude() const;```  
Convert complex values to their magnitude and store in vReal. Uses vReal and vImag.
* ```void compute(FFTDirection dir) const;```  
Calcuates the Fast Fourier Transform. Uses vReal and vImag.
* ```void dcRemoval() const;```  
Removes the DC component from the sample data. Uses vReal.
* ```T majorPeak() const;```  
Returns the frequency of the biggest spike in the analyzed signal. Uses vReal.
* ```void majorPeak(T &frequency, T &value) const;```  
Returns the frequency and the value of the biggest spike in the analyzed signal. Uses vReal.
* ```uint8_t revision() const;```  
Returns the library revision.
* ```void setArrays(T *vReal, T *vImag);```  
Replace the data array pointers.
* ```void windowing(FFTWindow windowType, FFTDirection dir, bool withCompensation = false);```  
Performs a windowing function on the values array. Uses vReal. The possible windowing options are:
  * Rectangle
  * Hamming
  * Hann
  * Triangle
  * Nuttall
  * Blackman
  * Blackman_Nuttall
  * Blackman_Harris
  * Flat_top
  * Welch

  If `withCompensation` == true, the following compensation factors are used:
  * Rectangle: 1.0 * 2.0
  * Hamming: 1.8549343278 * 2.0
  * Hann: 1.8554726898 * 2.0
  * Triangle: 2.0039186079 * 2.0
  * Nuttall: 2.8163172034 * 2.0
  * Blackman: 2.3673474360 * 2.0
  * Blackman Nuttall: 2.7557840395 * 2.0
  * Blackman Harris: 2.7929062517 * 2.0
  * Flat top: 3.5659039231 * 2.0
  * Welch: 1.5029392863 * 2.0

## Special flags

You can define these before including arduinoFFT.h:

* #define FFT_SPEED_OVER_PRECISION  
Define this to use reciprocal multiplication for division and some more speedups that might decrease precision.

* #define FFT_SQRT_APPROXIMATION  
Define this to use a low-precision square root approximation instead of the regular sqrt() call. This might only work for specific use cases, but is significantly faster. Only works if `T == float`.

See the `FFT_speedup.ino` example in `Examples/FFT_speedup/FFT_speedup.ino`.

# Migrating from 1.x to 2.x

* The function signatures where you could pass in pointers were deprecated and have been removed. Pass in pointers to your real / imaginary array in the ArduinoFFT() constructor. If you have the need to replace those pointers during usage of the library (e.g. to free memory) you can do the following:

```C++
const uint32_t nrOfSamples = 1024;
auto real = new float[nrOfSamples];
auto imag = new float[nrOfSamples];
auto fft = ArduinoFFT<float>(real, imag, nrOfSamples, 10000);
// ... fill real + imag and use it ...
fft.compute();
fft.complexToMagnitude();
delete [] real;
// ... replace vReal in library with imag ...
fft.setArrays(imag, nullptr);
// ... keep doing whatever ...
```
* All function names are camelCase case now (start with lower-case character), e.g. "windowing()" instead of "Windowing()".

## TODO
* Ratio table for windowing function.
* Document windowing functions advantages and disadvantages.
* Optimize usage and arguments.
* Add new windowing functions.
* ~~Spectrum table?~~
