/*
  gLCD Library Example 1:
  
  This example shows how to use the basic functions: Line, Plot, Box, Circle, Print, SetForeColour, and SetBackColour.
  
  gLCD should work for all Nokia Clone 128x128 screens which have the
  Epson or Phillips controller chip.
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
//gLCD graphic(RST,CS,Clk,Data,NORMAL_SPEED); //Normal speed
//For high speed, use:
gLCD graphic(RST,CS,Clk,Data,HIGH_SPEED); //High speed


void setup() {  
  /*Display needs to be initialised. You only need to do this once,
    You may have to press the Arduino reset button after uploading
    your code as the screen may fail to startup otherwise
    
    The first two variables in the begin() call specify where on the screen
    origin is. On some screens the first one or two rows/cols of
    pixels aren't visible, so begin(X,Y,,) allows all X and Y co-ords
    to be shifted slightly.
    
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
    
	
	To determine the offset (x,y) coordinate, run this example sketch. It will print a 10px x 10px red box
	in the top left corner of the screen. If that boxes outline does not line up with the corner, try
	using the offsets. A positive x will shift the screen to the right, negative x to the left. A positve Y
    will shift the display down, and a negative Y will shift it up.	
	
	
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
  
  graphic.setBackColour(GLCD_LIME); //set the background to lime colour.
  graphic.Clear();
  
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
  /*Lets draw a box...
  
    First we set the background colour and foreground colour
    We use the SetBackColour function to do this.
    The function takes three bytes, which are: Red, Green, Blue. 
    Each of these is a 4 bit number where 0 = off, 15 = fully on.
  */
    
  //We want a white background, so that would be 15 for each variable
  graphic.setBackColour(15,15,15);
  
  //We want a red foreground, so red will be 15 and the others 0
  graphic.setForeColour(15,0,0); 
  
  /*Now We draw our box. Lets use the Box() function.
    Box( X1, Y1, X2, Y2, format) takes 5 variables. 
    
    The first two are: X1, Y1. These are bytes which represent the (x,y) co-ord where the box will start
    
    The second two are: X2, Y2. These bytes represent the (x,y) co-ord where the box will end. NOTE: This coodinate will also be included in the box
    
    The final one is: format. This controls how the box will look (b7..b3 are ignored)

						  b2 | b1 | b0 | Meaning
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  1 |  x | Draws a box with just a border of colour BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  1 |  x | Draws a box with just a border of colour ForeColour
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  0 |  0 | Draws a box with border in BackColour and fill inside it with BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  0 |  0 | Draws a box with border in ForeColour and fill inside it with BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  0 |  1 | Draws a box with border in BackColour and fill inside it with ForeColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  0 |  1 | Draws a box with border in ForeColour and fill inside it with ForeColour
  
	Or you can use one of the predefined constants:
	OutlineBackColour = 0b010
	OutlineForeColour = 0b110
	SolidBackColour = 0b000
	BorderForeColour_FillBackColour = 0b100
	BorderBackColour_FillForeColour = 0b001
	SolidForeColour = 0b101
	
	
    Lets draw a box which starts from (10,10) and is 100 pixes square. So, X1 = 10, 
                                                                           Y1 = 10,
                                                                           X2 = 10 + 99,   <- Because X2,Y2 will be part of the box
                                                                           Y2 = 10 + 99 
    It will have a border coloured Red, and be filled with the background colour. So, 'format' = 4	
  */
  graphic.Box(10,10,109,109,4);
  //Or This:
  //graphic.Box(10,10,109,109,BorderForeColour_FillBackColour);
  
  //Inside is another box filled in the foreground colour
  graphic.Box(20,20,99,99,5);
  //Or This:
  //graphic.Box(20,20,99,99,SolidForeColour);
  
  /* Next we will write Hello in the centre. For this we need to use the print() function. This behaves exactly like Serial.print() only it prints to the screen not the
    serial port.
    
    First though we need to setup the text. There are two functions to do this.
    setCoordinate(X,Y);
    and
    setFont(Font);
                                                                           
    In setCoordinate(X,Y), we set the co-ordinates of where the text will be placed on the screen - (X,Y) pixels denotes the top left corner. 
                                                  NOTE: Currently there is no text wrap so strings that dont fit on the screen will be clipped. 
                                                        However you can use a '\n' character to print the characters following it on a new line directly below.
    
    Then, in setFont(Font), we set the Font. This controls the vertical and horizontal scaling of the text, and also whether it has a solid or transparent background.
                                                                          Font bits have the following meaning: (b7..b4 are ignored - maybe future use)
								
									  b2 | b2 | b1 | b0 | Meaning
									 ----+----+----+----+-----------------------------------------------
									   0 |  x |  x |  x | Text will be truncated if too long for screen
									 ----+----+----+----+-----------------------------------------------
									   1 |  x |  x |  x | Text will be wrapped onto new line if too long
									 ----+----+----+----+-----------------------------------------------
									   x |  0 |  x |  x | Text has transparent background
									 ----+----+----+----+-----------------------------------------------
									   x |  1 |  x |  x | Text has solid background
									 ----+----+----+----+-----------------------------------------------
									   x |  x |  0 |  0 | Text normal. (6 x 8 pixels)
									 ----+----+----+----+-----------------------------------------------
									   x |  x |  0 |  1 | Text Wide. (12 x 8 pixels)
									 ----+----+----+----+-----------------------------------------------
									   x |  x |  1 |  0 | Text Tall. (6 x 16 pixels)
									 ----+----+----+----+-----------------------------------------------
									   x |  x |  1 |  1 | Text Tall and Wide. (12 x 16 pixels)
	
	Or you can use one of the predefined constants:
	Normal_ClearBG = 0b000 = 0
	Wide_ClearBG = 0b001 = 1
	Tall_ClearBG = 0b010 = 2
	Large_ClearBG = 0b011 = 3
	Normal_SolidBG = 0b100 = 4
	Wide_SolidBG = 0b101 = 5
	Tall_SolidBG = 0b110 = 6
	Large_SolidBG = 0b111 = 7
	Normal_ClearBG_Wrap = 0b1000 = 8
	Wide_ClearBG_Wrap = 0b1001 = 9
	Tall_ClearBG_Wrap = 0b1010 = 10
	Large_ClearBG_Wrap = 0b1011 = 11
	Normal_SolidBG_Wrap = 0b1100 = 12
	Wide_SolidBG_Wrap = 0b1101 = 13
	Tall_SolidBG_Wrap = 0b1110 = 14
	Large_SolidBG_Wrap = 0b1111 = 15
	
    We then finally call the print function with our text or variable, e.g.
    graphic.print("Some Text");
        See print() in the Arduino Reference for more details.
  */
  graphic.setForeColour(0,0,15); //Text is coloured Blue
  graphic.setFont(4); //normal size text, solid background.
  //graphic.setFont(Normal_SolidBG); //normal size text, solid background.
  graphic.setCoordinate(40,40);
  graphic.print("Hello\nWorld"); //Hello and World will be printed on seperate lines as governed by the newline character '\n'
  
  graphic.setFont(12); //normal size text, solid background. Wrap string to new line if needed
  //graphic.setFont(Normal_SolidBG_Wrap); //normal size text, solid background. Wrap string to new line if needed
  graphic.setCoordinate(0,112);
  graphic.print("This string is too long for a single line"); //The string is too long for a single line, but by selecting one of the '_Wrap' fonts, it will be automatically split onto new lines.
  
  /* Now lets double underline the text twice. Each normal character takes up 6 x 8 pixels, so we need to draw a line 8+9 pixels below the top of the text as there are two lines.
     The line is 6*5 = 30px long. And a second of the same length which is 11 pixels below the top of the text.
     
     For this we can use the Line() Function.
     Line(X1,X2,Y1,Y2,Colour) takes five variables
     
     Firstly X1 and Y1 are both unsigned chars which are where the line starts.
     
     The second two are: X2,Y2. These are the last point on the line.
     
     Lastly: Colour. This defines whether the line should be in Foreground colour (Colour = 1) or Background colour (Colour = 0)
  */
  graphic.Line(40,57,69,57,4); // The lines will be Blue as that was the last foreground colour that was set.
  graphic.Line(40,59,69,59,4);
  
  /* The next function is Circle().
     Circle(X1,Y1,Radius,Format) takes four variables 
    
    The first two are: X1, Y1. These are bytes which represent the (x,y) co-ord of the centre of the circle.
    
    The next is: Radius. This is the radius of the circle in pixels.
    
    The final one is: Format.  This controls how the Circle will look (b7..b3 are ignored)

						  b2 | b1 | b0 | Meaning
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  1 |  x | Draws a Circle with just a border of colour BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  1 |  x | Draws a Circle with just a border of colour ForeColour
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  0 |  0 | Draws a Circle with border in BackColour and fill inside it with BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  0 |  0 | Draws a Circle with border in ForeColour and fill inside it with BackColour
						 ----+----+----+-----------------------------------------------------------------------------
						   0 |  0 |  1 | Draws a Circle with border in BackColour and fill inside it with ForeColour
						 ----+----+----+-----------------------------------------------------------------------------
						   1 |  0 |  1 | Draws a Circle with border in ForeColour and fill inside it with ForeColour
	Or you can use one of the predefined constants:
	OutlineBackColour = 0b010
	OutlineForeColour = 0b110
	SolidBackColour = 0b000
	BorderForeColour_FillBackColour = 0b100
	BorderBackColour_FillForeColour = 0b001
	SolidForeColour = 0b101
  */
  graphic.Circle(66,74,15,4); //Draw circle of radius 10px, filled with the background colour 
  //Or:  
  //graphic.Circle(66,74,15,BorderForeColour_FillBackColour); //Draw circle of radius 10px, filled with the background colour 
  
  /* The last basic function is Plot(). This simply sets the colour of the pixel at (x,y) to the current foreground colour.
     Plot(X,Y) takes three variables.
     
     X and Y are the co-odinate where the dot is drawn.
          
     Colour defines whether the line should be in Foreground colour (Colour = 3) or Background colour (Colour = 0)
  */
  for (int i = 15;i < 19;i++){
    for (int j = 17;j < 21;j++){
      graphic.Plot((i*4), (j*4),3); //Draw a grid of 16 dots.
    }
  } 
   
  while(1); //Finished
}
