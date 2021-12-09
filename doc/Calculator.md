# Calculator Manual
This is a simple Calculator with support for the four basic arithmetic operations, parenthesis and exponents.
Here is what you need to know to make full use of it:
- Swipe left to access parenthesis and exponents:
   ![](./ui/calc2.jpg)
- A long tap on the screen will reset the text field to `0`.
- If the entered term is invalid, the watch will vibrate.
- The calculator uses 32bit floating point numbers internally and doesn't check for over- or underflows.
because of this ![](./ui/calc3.jpg) will result in ![](./ui/calc4.jpg)
 - results are rounded to 4 digits after the decimal point
- **TIP:** you can use `^(1/2)` to calculate square roots