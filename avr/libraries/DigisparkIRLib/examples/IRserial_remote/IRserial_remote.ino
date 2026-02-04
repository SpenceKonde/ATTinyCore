#include <IRLib.h>

IRsend My_Sender;

int protocol;
long code;
int bits;
void setup() {
  Serial.begin(9600);
}

long parseHex (void) {
  long Value=0; char C;delay(100);
  while (Serial.available()>0) {
     C= tolower(Serial.read());
     if ((C>='0')&&(C<='9'))
       C=C-'0';
     else
       if ((C>='a') && (C<='f'))
         C=C-'a'+10;
       else
         return Value;
     Value=  C+(Value<<4);
  };
  return Value;
}
void parseDelimiter () {
  char C;
  while(Serial.available()>0) {
    C=tolower(Serial.peek());
    if( (C>='0') && (C<='9') )return;
    if( (C>='a') && (C<='f') )return;
    C=Serial.read();//throwaway delimiters
    delay (5);
  }
}
//     enum IRTYPES {UNKNOWN, NEC, SONY, RC5, RC6, PANASONIC_OLD, JVC, NECX, HASH_CODE, LAST_PROTOCOL=HASH_CODE};

void loop() {
  if (Serial.available ()>0) {
    protocol = Serial.parseInt (); parseDelimiter();
    code     = parseHex ();        parseDelimiter();
    bits     = Serial.parseInt (); parseDelimiter();
/*  Serial.print("Prot:");  Serial.print(protocol);
    Serial.print(" Code:"); Serial.print(code,HEX);
    Serial.print(" Bits:"); Serial.println(bits);
  */
    My_Sender.send(IRTYPES(protocol), code, bits);   
  }
}
