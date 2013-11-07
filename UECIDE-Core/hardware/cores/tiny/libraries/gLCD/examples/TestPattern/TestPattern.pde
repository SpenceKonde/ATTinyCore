/*
  gLCD Library Example 2:
  
  This example shows you how to create an instance of the display,
  connect it to the correct pins, and then display the built in
  Test Pattern.
  
  gLCD should work for all Nokia Clone 128x128 screens which have the
  Epson controller chip.
*/

#include <gLCD.h>

//You can use these variables to set the pin numbers
const char RST = 8;
const char CS = 9;
const char Clk = 13;
const char Data = 11;

/*Create an instance of the display. Lets call it 'graphic'.
  There are four variables,
  which define which pins the LCD is connected too. 
  these are:
    Reset, Chip Select, Clock, Data.
  
  A fifth variable 'speed' can be included as 0 or 1, which controls whether to enter high speed mode (see below).
  If the fifth variable is omitted (i.e. only the first four are given), normal speed mode is implied.

  gLCD graphic(RST,CS,Clk,Data [,speed]);

  Note, it is also possible to enable a high speed mode which increases the speed of the display
  by a factor of >5. This is done through the library bypassing the digitalWrite function and
  directly addressing port registers. 
  For the Due, High speed mode increases speed to such an extent that this example completes before you can blink.
  
  Please note that while this doesn't affect how you use the library (it is all handled internally),
  it is possible that it may run too fast for your display and thus the display will appear to not work. 
  This is UNLIKELY to happen, and I have checked with several displays which all worked fine.
  
  As of version 2.4 nothing special needs to be done to use this high speed mode, so I am now mentioning
  it is the example file.
  
*/
//For normal speed, use:
gLCD graphic(RST,CS,Clk,Data,NORMAL_SPEED); //Normal speed

//For high speed, use:
// gLCD graphic(RST,CS,Clk,Data,HIGH_SPEED); //High speed


void setup() {  
 /*Display needs to be initialised. You only need to do this once,
   You may have to press the Arduino reset button after uploading
   your code as the screen may fail to startup otherwise
   
   The first two variables in the begin() call specify where on the screen
   origin is. On some screens the first one or two rows/cols of
   pixels aren't visible, so begin(X,Y,,) allows all X and Y co-ords
   to be shifted slightly. (0,2) works well for both my screens.
   
   The third variable specifies whether the colours are inverted.
   If the White background is Black on your screen, set this to a 1
   Else leave it as a zero
   
   The fourth variable is the driver to use you can choose from. There are 6 drivers:
   EPSON (or) EPSON_4 (both are the same) 
   EPSON_5
   PHILLIPS_0
   PHILLIPS_1
   PHILLIPS_2
   PHILLIPS_3
   
   For an Epson Screen:
 */
 
 //This
 graphic.begin(0,0,0,EPSON); //Normal Epson
 //or
 //graphic.begin(0,0,0,EPSON_4); //Normal Epson
 //or
 //graphic.begin(0,0,0,EPSON_5); //16bit mode Epson - suits some rare screens.
 
 //For a Phillips Screen:
 //This
 //graphic.begin(0,0,0,PHILLIPS_0); //Normal X direction
 //or
 //graphic.begin(0,0,0,PHILLIPS_1); //Revesed X direction.
 //or
 //graphic.begin(0,0,0,PHILLIPS_2); //Normal X direction, Mirrored.
 //or
 //graphic.begin(0,0,0,PHILLIPS_3); //Revesed X direction, Mirrored.
 
 
 
 /*If you can't see the colours properly, then uncomment the 
   graphic.Contrast() function. 
   If the Screen is too dark, or you cant see anything send a value >0x2B (>0x30 for Phillips). If you cant see all
   the different colours distinctly on the test pattern, send a value <0x2B (<0x30 for Phillips). The default values
   should work for many screens, so you may not have to use this at all.
   
   Range for phillips: -0x3F to 0x3F
   Range for EPSON:		0x00 to 0x3F
 */

 //graphic.Contrast(0x2B);
}
void loop() {
  //Print out the test pattern.
  graphic.testPattern();
  while(1); //Finished
}
