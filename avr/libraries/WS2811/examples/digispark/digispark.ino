#include <WS2811.h>
DEFINE_WS2811_FN(WS2811RGB, PORTB, 1)
RGB_t rgb[1]; //1 for 1 pixel

void setup() {                

  pinMode(1,OUTPUT);

}

void loop() {
	setPixel(0,255,0,0); //set first pixel (zero indexed) to red
	updatePixels(); //show the change
}

void setPixel(i,r,g,b){
	rgb[r].r=r;
    rgb[g].g=g;
    rgb[b].b=b;
}

void updatePixels(){
	WS2811RGB(rgb, ARRAYLEN(rgb));
}