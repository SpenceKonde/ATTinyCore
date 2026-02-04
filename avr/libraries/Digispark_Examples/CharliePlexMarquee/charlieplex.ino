// where does our characterMap start in the ASCII code
#define MAP_START      32

#define DISPLAY_WIDTH  4
#define DISPLAY_HEIGHT 5

// "pixels" per second
#define SPEED          10

// the text to display
#define DISPLAY_STRING "HELLO WORLD!"


// maps characters to their 4x5 grid 
unsigned long characterMap[59];

// set up a character in the characterMap
void Chr(char theChar, unsigned long value) {
  characterMap[theChar - MAP_START] = value;
}

// The offset of our string in the display
int offset = 0;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;
unsigned int timeout;

char myString[] = DISPLAY_STRING;
int length = sizeof(myString);

// render the string on the given offset
void renderString(char *theString, int offset) {
  int index = 0;
  while (theString[index]) {
    renderCharacter(theString[index], offset - index * (DISPLAY_WIDTH + 1));
    index++;
  }
}

// render a character on the given offset
void renderCharacter(char theChar, int charOffset) {
  if (charOffset <= -DISPLAY_WIDTH || charOffset > DISPLAY_WIDTH) {
    // off the 'screen' nothing to do
    return;
  }

  unsigned long graphic = characterMap[theChar - MAP_START];

  for (byte y = 0; y < DISPLAY_HEIGHT; y++) {
    for (byte x = 0; x < DISPLAY_WIDTH; x++) {
      if (graphic & 0x1) {
        // 3 - x to reverse order
        lightPixel(3 - x - charOffset, y);
      }
      graphic = graphic >> 1;
    }
  }
}

// light a pixel at the given coordinates
void lightPixel(byte x, byte y) {
  if (x >= 0 && x < DISPLAY_WIDTH) {
    if (y <= x) {
      x++;
    }
    LEDon(y, x);
  }
}

// turn on the pins to light a LED
void LEDon(byte vin, byte gnd) {
  delay(1);
  pinMode(0, INPUT); 
  pinMode(1, INPUT); 
  pinMode(2, INPUT); 
  pinMode(3, INPUT); 
  pinMode(4, INPUT); 

  pinMode(vin, OUTPUT);   
  pinMode(gnd, OUTPUT); 
  digitalWrite(vin, HIGH);
  digitalWrite(gnd, LOW); 
}

// runs at start
void setup() {
  // set up render map

  // Rows:   1---2---3---4---5---
  Chr('A', 0b01101001111110011001);
  Chr('B', 0b11101001111010011110);
  Chr('C', 0b01111000100010000111);
  Chr('D', 0b11101001100110011110);
  Chr('E', 0b11111000111010001111);
  Chr('F', 0b11111000111010001000);
  Chr('G', 0b01111000101110010110);
  Chr('H', 0b10011001111110011001);
  Chr('I', 0b01110010001000100111);
  Chr('J', 0b01110010001010100100);
  Chr('K', 0b10011010110010101001);
  Chr('L', 0b10001000100010001111);
  Chr('M', 0b10011111111110011001);
  Chr('N', 0b10011101101110011001);
  Chr('O', 0b01101001100110010110);
  Chr('P', 0b11101001111010001000);
  Chr('Q', 0b01101001101101100001);
  Chr('R', 0b11101001111010101001);
  Chr('S', 0b11111000111100011111);
  Chr('T', 0b01110010001000100010);
  Chr('U', 0b10011001100110010110);
  Chr('V', 0b10011001100110100100);
  Chr('W', 0b10011001111111110110);
  Chr('X', 0b10011001011010011001);
  Chr('Y', 0b10011001011000101100);
  Chr('Z', 0b11110001001001001111);
  Chr(' ', 0b00000000000000000000);
  Chr('!', 0b01000100010000000100);

  // how long to wait between shifting the display
  timeout = 1000 / SPEED;
}

// loops continuously
void loop() {
  currentMillis = millis();

  renderString(myString, offset);

  if (currentMillis - lastMillis > timeout) {
    lastMillis = currentMillis;
    // shift string over one "pixel"
    offset++;
    // if it's past the length of the string, start over from the beginning
    if (offset > length * (DISPLAY_WIDTH + 1)) {
      offset = -DISPLAY_WIDTH;
    }
  }
}