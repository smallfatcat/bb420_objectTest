#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h> // includes the LiquidCrystal Library
#include "bb420_headers.h"
LiquidCrystal_I2C  lcd(0x3F,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified backpack

// Stepper Pins
#define stepPin 8
#define dirPin 7
#define enablePin 9
#define mode0Pin 12
#define mode1Pin 11
#define mode2Pin 10

//Button Pins
#define buttonAPin 3
#define buttonBPin 4
#define buttonCPin 5
#define buttonDPin 6

Controller mainControl(buttonAPin, buttonBPin, buttonCPin, buttonDPin, 300, 0, 30);

// LCD vars
String LCDbuffer0 = "";
String LCDbuffer1 = "";
int frameCount = 0;

void setup() {
  //setup Timer1
  cli();
  TCCR1A = 0b00000000;
  TCCR1B = 0b00001001;        // set prescalar to 1
  TIMSK1 |= 0b00000010;       // set for output compare interrupt
  //setMotorSpeed(manSpeed);
  mainControl.motor1.setSpeed(300); 
  sei();                      // enables interrupts. Use cli() to turn them off
  
  // Setup LCD Module
  lcd.begin(16,2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  //createChars();
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.noCursor();
  
  Serial.begin(9600);
  mainControl.butA.debug();
  mainControl.butB.debug();
  mainControl.butC.debug();
  mainControl.butD.debug();
  Serial.println("Ready");
  mainControl.motor1.activate();
  mainControl.motor1.setDirection(1);
}

void loop(){
  frameCount++;
  
  //mainControl.getButtonStates();
  mainControl.logic();
  //mainControl.serialOut();
  //mainControl.motor1.serialOut();

  if(frameCount%1000 == 0){
    // LCD TEST
    String line0 = mainControl.displayLCD0();
    String line1 = mainControl.displayLCD1();
    if( ( LCDbuffer0 != line0 ) || ( LCDbuffer1 != line1 ) ){
      LCDbuffer0 = line0;
      LCDbuffer1 = line1;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print( line0 );
      lcd.setCursor(0,1);
      lcd.print( line1 );
    }
  }
}

//
// ---------------------------- Timer --------------------------------------------------
ISR(TIMER1_COMPA_vect) {
  if(mainControl.motor1.isActive() ){
    if(mainControl.motor1.loopCount == 0){
      mainControl.motor1.loopCount = mainControl.motor1.targetLoopCount;
      digitalWrite(stepPin, HIGH);       // Driver only looks for rising edge
      digitalWrite(stepPin, LOW);        //  DigitalWrite executes in 16 us  
      if(mainControl.motor1.getDir() == UP){
        mainControl.motor1.pulseCount++;
      }
      else{
        mainControl.motor1.pulseCount--;
      }
    }
    else{
      mainControl.motor1.loopCount-- ;
    }
  }
}
/*
void setMotorSpeed(int newMotorSpeed){
  long timerCount = 16000000/newMotorSpeed - 1;
  if(timerCount < 65536){ 
    loopCount = 0;
    targetLoopCount = loopCount;
    OCR1A = timerCount;
  }
  else{
    loopCount = floor(timerCount / 65535);
    targetLoopCount = loopCount;
    OCR1A = round(timerCount / (loopCount+1));
  }
}
*/
