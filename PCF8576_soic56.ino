#include <Wire.h> //include Wire.h library
byte x = 0x00;
//
//The driver PCF8576 allow the control of 160 segments based in the 4 numberOfbackPlanes. BP0, BP1, BP2, BP3.
//
//Four different slave addresses can be chosen by connecting ADR either to VEE, 3/8 VCC, 5/8 VCC or VCC. This results in
//the corresponding valid addresses HEX 70, 72, 74 and 76 for writing and 71, 73, 75 and 77 for reading. All other
//addresses cannot be acknowledged by the circuit where we using a PCF8576 driver.
/*
Control bits presents at 2º byte
C0 = 0 static mode, i.e. continuous display of digits 1 and 2
C0 = 1 dynamic mode, i.e. alternating display of digit 1 + 3 and 2 + 4
C1 = 0/1 digits 1 + 3 are blanked/not blanked
C2 = 0/1 digits 2 + 4 are blanked/not blanked
C3 = 1 all segment outputs are switched-on for segment test(1)
C4 = 1 adds 3 mA to segment output current
C5 = 1 adds 6 mA to segment output current
C6 = 1 adds 12 mA to segment output current
*/

#define BUTTON_PIN 2 //Att check wich pins accept interrupts... Uno is 2 & 3
volatile byte buttonReleased = false;

bool flagSet = false;
bool flagReached = false;
uint8_t addr = 0x38;
uint8_t cnt = 0x00;

unsigned long lng32a, lngTmpa;
unsigned long lng32b, lngTmpb;
unsigned long lng32c, lngTmpc;
unsigned long lng32d, lngTmpd;
unsigned long lngTmp;

unsigned char word0 = 0;
unsigned char word1 = 0;
unsigned char word2 = 0;
unsigned char word3 = 0;
unsigned char word4 = 0;
unsigned char word5 = 0;
unsigned char word6 = 0;
unsigned char word7 = 0;
unsigned char wordMixH = 0;
unsigned char wordMixL = 0;
unsigned char wordMix = 0;


void setup() {
  //Wire.setClock(100000); //Frquency used at communitation
  Wire.begin(); // Initialize I2C (Master Mode: address is optional)
  Serial.begin(115200);
  //  pinMode(18, OUTPUT); //SDA pin A4
  //  pinMode(19, OUTPUT); //SCL pin A5
  //initialize the LED pin as an output:
    pinMode(13, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),
                  buttonReleasedInterrupt,
                  FALLING);
    configDSP();
}
void configDSP(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  //Mode configuration of LCD
  //MODE SET: C 1 0 LP E B M1 M0
  //C=0; last command.
  //C=1; commands continue.
  //M1 M0 = 01 static, =10 2BP, =11 3BP, =00 4BP
  Wire.write(0x58);  // Here the value 0x48 put the bit "E" at value 1 because the "8"
 
  Wire.endTransmission();    // stop transmitting
  delay(50);
}
void cmdBlink(uint8_t blinkFreq){
  //The definition of blink and frequency is done on this command
  //I receive value between 1 and 3 which is: 0 is Off, 0.25Hz, 1 and 2Hz See table 13
      /*
        BLINK C 1 1 1 0 A BF1 BF0 Table 13 Defines the blinking frequency.
        Table 14 Selects the blinking mode; normal operation with
        frequency set by BF1, BF0 or blinking by alternation of
        display RAM banks. Alternation blinking does not
        apply in 1 : 3 and 1 : 4 multiplex drive modes.
      */
    Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
    //00000 SC,SB,SA
    //Wire.write(0xCC);  //bit 7 is 0:last command, 1:continue command
    //Wire.write(0b01000111); //The first byte set as Auto-Increment and allow the second byte as control status
    Wire.write(0x70 | blinkFreq); 
    //Wire.write(0x8F);  //bit 7 is 0:last command, 1:continue command
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void cmdBitMemory(uint8_t bitAddress){
    /*
      LOAD DATA POINTER
      C 0 P5 P4 P3 P2 P1 P0 Table 9 Six bits of immediate data, bits P5 to P0, are
      transferred to the data pointer to define one of forty
      display RAM addresses.
    */
    Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
    //00000 SC,SB,SA
    //Wire.write(0xCC);  //bit 7 is 0:last command, 1:continue command
    //Wire.write(0b01000111); //The first byte set as Auto-Increment and allow the second byte as control status
    Wire.write(0b00000000 | bitAddress); 
    //Wire.write(0x8F);  //bit 7 is 0:last command, 1:continue command
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void wrSegments(uint8_t segments){
    Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
    //00000 SC,SB,SA
    //Wire.write(0xCC);  //bit 7 is 0:last command, 1:continue command
    //Wire.write(0b01000111); //The first byte set as Auto-Increment and allow the second byte as control status
    Wire.write(0b00000000 | segments); 
    //Wire.write(0x8F);  //bit 7 is 0:last command, 1:continue command
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void tstSegmentsOn(){
    Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
    Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
    Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
    Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
    Wire.write(byte(0b11111000));   //Bank Select
    Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    //
    // Wire.write(0x8F); Wire.write(0x80); Wire.write(0x80); Wire.write(0x80); Wire.write(0x80);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // //Wire.write(0x8F); Wire.write(0x8F); Wire.write(0x8F); Wire.write(0x8F); Wire.write(0x8F);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void tstSegmentsOff(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
    //
    Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime0(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime1(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100100); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime2(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b01000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime3(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00100000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime4(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100010); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime5(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000110); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime6(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b01100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void anime7(){
  Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
    //        D17~D10;           D27~D20            D37~D30            D47~D40
    //1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
    Wire.write(0b00000000); //  7:0 
    Wire.write(0b00000100); // 15:8     Wire.write(0b00000110); // 15:8
    Wire.write(0b00100000); // 23:16    Wire.write(0b01100110); // 23:16 
    Wire.write(0b00000000); // 31:24    Wire.write(0b11100000); // 31:24
    Wire.write(0b00000000); // 39:32
    Wire.write(0b00000000); // 47:40
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000); 
    Wire.write(0b00000000);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
    
    Wire.endTransmission();    // stop transmitting
    delay(5);
}
void ctrlBright(uint8_t miliamp){
  Wire.beginTransmission(addr); // transmit to device #?
  // 00000 SC,SB,SA
  miliamp = (0b00010111 | miliamp);
  Wire.write(0x00);  Wire.write(miliamp); //The first byte set as Auto-Increment and allow the second byte as control status
  //        D17~D10;           D27~D20            D37~D30            D47~D40
  //  1ºdigit:   hgfedcba    2ºdigit:   hgfedcba 3ºdigit:   hgfedcba  4ºdigit:   hgfedcba
  //Wire.write(msgHello[i]);  Wire.write(msgHello[i]);  Wire.write(msgHello[i]);  Wire.write(msgHello[i+1]);
  //Wire.write(msgNumbers[8]);  Wire.write(msgNumbers[8]);  Wire.write(msgNumbers[8]);  Wire.write(msgNumbers[8]);  
  Wire.endTransmission();    // stop transmitting
  delay(500);
}
void ledBlink(){
   for(uint8_t led = 0x00; led < 5; led++){
    digitalWrite(13, LOW);
    delay(200);
    digitalWrite(13, HIGH);
    delay(200);
  }
}
void clearLCD(){
   //The table is 4rows by 40 columns
  Wire.beginTransmission(addr);   // transmit to device 0B01110000
                                  // device address is specified in datasheet
  Wire.write(byte(0b11001100));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
  Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
  Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
  Wire.write(byte(0b11111000));   //Bank Select
  Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0

  for(uint8_t b=0; b<20; b++){
    Wire.write(byte(0b00000000)); 
  }
   //Wire.write(val);              // sends potentiometer value byte  
  Wire.endTransmission();         // stop transmitting
}
void tst(uint8_t grp){
  uint8_t group=0;
  uint8_t nMask = 0x00;
  group = grp;
  byte Aa,Ab,Ac,Ad,Ae,Af,Ag,Ah;
  
    Aa=0x00; Ab=0x00; Ac=0x00; Ad=0x00; Ae=0x00; Af=0x00; Ag=0x00; Ah=0x00; 
  
                          Wire.beginTransmission(addr); // transmit to device #? //Find on PCF8576 the address of 0x38 and 0x50
                          //The M1 & M0 is defined to work only to 80 segments at line below because the LCD only have 80segments!!!
                          Wire.write(byte(0b11001010));   //Mode Set, sends instruction byte to define static or 1:2~1:4, M1,M0
                          Wire.write(byte(0b10000000));   //Load data pointer Last 6 bits: P5,P4,P3,P2,P1,P0
                          Wire.write(byte(0b11100000));   //Table 9 Device Select, Last 3 bits: A2,A1,A0
                          Wire.write(byte(0b11111000));   //Bank Select
                          Wire.write(byte(0b01110000));   //BLINK, Last 3 bits A, BF1, BF0
                          delay(1);
                              switch (group){
                                case 0: Aa=nMask;
                                  Wire.write(~(word0)); Wire.write(~(word1)); Wire.write(~(word2)); Wire.write(~(word3)); Wire.write(~(wordMix));
                                  Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00));
                                  // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                  // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                  Serial.println(group, DEC);
                                 break;
                                case 1: Ab=nMask;
                                  Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00)); Wire.write(~(0x00));
                                  Wire.write(~(word0)); Wire.write(~(word1)); Wire.write(~(word2)); Wire.write(~(word3)); Wire.write(~(wordMix));
                                  // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                  // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                  Serial.println(group, DEC);
                                 break;
                                // case 2: Ac=nMask;
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Wire.write(0xAa); Wire.write(0xAb); Wire.write(0xAc); Wire.write(0xAd); Wire.write(0xAe);
                                //   Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Serial.println(group, DEC);
                                //  break;
                                // case 3: Ad=nMask;
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Wire.write(0xAa); Wire.write(0xAb); Wire.write(0xAc); Wire.write(0xAd); Wire.write(0xAe);
                                //   Serial.println(group, DEC);
                                //  break;
                                // case 4: Ae=nMask;
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   // Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00); Wire.write(0x00);
                                //   Wire.write(0xAa); Wire.write(0xAb); Wire.write(0xAc); Wire.write(0xAd); Wire.write(0xAe);
                                //   Serial.println(group, DEC);
                                //  break; 
                              }
                                  // Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
                                  // Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
                                  //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
                                  //Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF); Wire.write(0xFF);
                           Wire.endTransmission();// stop transmitting
                           delayMicroseconds(5);
                           group++;
                           delay(500);                     
}
void findSeg(){
  unsigned long mask = 0x00000001;
  //lng32a = 0b0000 0000 0000 0000 0000 0000 0000 0001  //This is only as one example of representation of a long format binary
  //lng32a = 0x00000001 //This is only as one example of representation of a long format hexadecimal
  for (uint8_t grp = 0; grp< 2; grp++){  // The LCD have half of our capacity of control (160), this means I only need send 80 bits (10 bytes)
      for(uint8_t weight = 0; weight < 32; weight++){
        lngTmpa = mask << weight;
        lngTmpb = mask << weight;
            word3 = (lngTmpa & 0xff000000UL) >> 24;
            word2 = (lngTmpa & 0x00ff0000UL) >> 16;
            word1 = (lngTmpa & 0x0000ff00UL) >>  8;
            word0 = (lngTmpa & 0x000000ffUL) >>  0; 
            word7 = (lngTmpb & 0xff000000UL) >> 24;
            word6 = (lngTmpb & 0x00ff0000UL) >> 16;
            word5 = (lngTmpb & 0x0000ff00UL) >>  8;
            word4 = (lngTmpb & 0x000000ffUL) >>  0; 
            wordMixL = ((lngTmpa & 0xf0000000UL) >> 28);
            wordMixH = ((lngTmpb & 0x0000000fUL) >> 0);
            wordMixH = (wordMixH << 4);
            wordMix = (wordMixH | wordMixL);
        // Serial.print(wordMix, BIN); Serial.print( " , "); Serial.println(weight, DEC);
        // delay(500);
        while(1){
                                            if(!buttonReleased){
                                              delay(200);
                                              //break; //Comment this line to respect the action of button!
                                            }
                                            else{
                                              delay(15);
                                              buttonReleased = false;
                                              break;
                                              }
                                    }
        tst(grp);
        Serial.print("seg nº"); Serial.println(cnt, DEC);
        cnt++;
      }
  }
}
void animation(){
    for(uint8_t s =0; s < 10; s++){
    //anime0(); delay(200);
    anime1(); delay(200);
    anime2(); delay(200);
    anime3(); delay(200);
    anime4(); delay(200);
    anime5(); delay(200);
    anime6(); delay(200);
    // anime7(); delay(200);
    }
}
void loop() {
  uint8_t current=0b00010000; //The bits to control the current are the bit: 4, 5 and 6.
  uint8_t bitMemory = 0x00;
  //Next for determine frequency of blinking
    for(uint8_t i = 0x01; i < 4; i++){
        cmdBlink(i);
        delay(2000);
    }
  //Here it is defined without blinking
   cmdBlink(0x00);

  clearLCD();
  tstSegmentsOn();
  delay(750);
  tstSegmentsOff();
  delay(750);
  tstSegmentsOn();
  //findSeg(); // Uncomment this line to procedure with identification of segments order!
    clearLCD();
    animation();
    delay(1000);
    // //The next for is to control the bright, it is available with 3 levels.
     tstSegmentsOn();
         for(uint8_t mili = 0; mili < 3; mili++){
         current = (0b00000000 << mili);
         ctrlBright(current);
         Serial.println(mili, HEX);
         delay(800);
    }
}
void buttonReleasedInterrupt() {
  buttonReleased = true; // This is the line of interrupt button to advance one step on the search of segments!
}
