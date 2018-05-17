#include <Arduino.h>
#include "bb420_headers.h"

//*********************************************************************
// Motor Methods
Motor::Motor () {
  speed     = 300;
  direction = 0;
  active = false;
  position = 0;
}

Motor::Motor (long initSpeed, int initDirection) {
  speed     = initSpeed;
  direction = initDirection;
}

int Motor::setSpeed(int newSpeed){
  speed = newSpeed;
  long timerCount = 16000000/newSpeed - 1;
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
  return speed;
}

int Motor::setDirection(int newDirection){
  direction = newDirection;
  return direction;
}

int Motor::setPosition(long newPosition){
  position = newPosition;
  return position;
}

bool Motor::activate(){
  active = true;
  return active;
}

bool Motor::deactivate(){
  active = false;
  return active;
}

bool Motor::serialOut(){
  Serial.print("Active: ");
  Serial.print(active ? "true" : "false");
  Serial.print(" speed: ");
  Serial.print(speed);
  Serial.print(" direction: ");
  Serial.print(direction);
  Serial.print(" pulseCount: ");
  Serial.print(pulseCount);
  Serial.print(" position ");
  Serial.println(position);
}

//*********************************************************************
// Button Methods
Button::Button () {
  s = HIGH;
}

Button::Button (int buttonPin) {
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP);
  s = HIGH;
}

bool Button::buttonInit (int buttonPin) {
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP);
  s = HIGH;
  return true;
}

bool Button::debug () {
  Serial.print("Button pin: ");
  Serial.println(pin);
}

bool Button::updateButton(){
  int currentState = digitalRead(pin);
  if(currentState != lastState){
    debounceTimer = millis();
  }
  if(millis()-debounceTimer > debounceDelay){
    if(currentState != s){
      s = currentState;
    }
  }
  lastState = currentState;

  // Repeat delay code
  if(s==LOW){
    if(reset){
      reset = false;
      pressedTime = millis();
      return true;
    }
    else if((millis()-pressedTime)>BUT_DELAY){
      pressedTime += BUT_REPEAT;
      return true;
    }
    
  }
  if(s==HIGH){
    reset = true;
  }
  return false;
}

//*********************************************************************
// Controller Methods

Controller::Controller (int A, int B, int C, int D, int L1, int L2, int speed, int direction, unsigned long d) {
  mode = 0;
  butA.buttonInit(A);
  butB.buttonInit(B);
  butC.buttonInit(C);
  butD.buttonInit(D);
  limit1.buttonInit(L1);
  limit2.buttonInit(L2);
  motor1.setSpeed(speed);
  motor1.setDirection(direction);
  BB_timer.delayActive = false;
  BB_timer.delayStart = millis();
  BB_timer.delayTime = d;
}

bool Controller::getButtonStates() {
  Serial.print("ButtonStates: ");
  if(butA.updateButton()){
    Serial.print("A");
  }
  if(butB.updateButton()){
    Serial.print("B");
  }
  if(butC.updateButton()){
    Serial.print("C");
  }
  if(butD.updateButton()){
    Serial.print("D");
  }
  Serial.println("");
  return true;
}

int Controller::nextMode(){
  //Serial.println("Next");
  if(mode == MAX_MODES){
    mode = 0;
  }
  else{
    mode++;
  }
  //Serial.println(mode);
  return mode;  
}

bool Controller::logic(){
  bool A = butA.updateButton();
  bool B = butB.updateButton();
  bool C = butC.updateButton();
  bool D = butD.updateButton();
  bool L1 = limit1.updateButton();
  bool L2 = limit2.updateButton();
  int Astate = butA.state();
  int Bstate = butB.state();
  int Cstate = butC.state();
  int Dstate = butD.state();
  int L1state = limit1.state();
  int L2state = limit2.state();

  if(mode != MODE_AUTO){
    motor1.deactivate();
  }
  if(A){
    nextMode();
    if(mode == MODE_DELAY){
      delayNew = BB_timer.delayTime;
      delayDirty = false;
    }
    if(mode == MODE_SPEED){
      speedNew = motor1.getSpeed();
      speedDirty = false;
    }
  }
  
     
  // Menus
  // Auto
  if(mode == MODE_AUTO){
    if(B){
      if( motor1.isActive() ){
        motor1.deactivate();
      }
      else{
        motor1.activate();
      }
    }
    if(C){
      motor1.setDirection(0);
    }
    if(D){
      motor1.setDirection(1);
    }
  }
  // Manual
  else if(mode == MODE_MAN){
    if(B){
      
    }
    if(Cstate == LOW){
      motor1.setDirection(0);
      motor1.activate();
    }
    if(Dstate == LOW){
      motor1.setDirection(1);
      motor1.activate();
    }
  }
  // Speed
  else if(mode == MODE_SPEED){
    if(B && speedDirty){
      speedDirty = false;
      motor1.setSpeed( speedNew );
      setMemFlag = true;
    }
    if(C){
      speedNew--;
      speedDirty = true;
    }
    if(D){
      speedNew++;
      speedDirty = true;
    }
  }
  // Delay
  else if(mode == MODE_DELAY){
    if(B && delayDirty){
      delayDirty = false;
      BB_timer.delayTime = delayNew;
      setMemFlag = true;
    }
    if(C){
      delayNew--;
      delayDirty = true;
    }
    if(D){
      delayNew++;
      delayDirty = true;
    }
  }
  return true;
}

bool Controller::serialOut(){
  String output;
  if(mode == MODE_AUTO){
    output = "Auto: ";
  }
  if(mode == MODE_MAN){
    output = "Manual";
  }
  Serial.print("Mode: ");
  Serial.println(output);
}

String Controller::displayLCD0(){
 String output;
  if(mode == MODE_AUTO){
    output = "Auto ";
    output += motor1.getDir() ? ">" : "<";
    output += " " + String(motor1.getPosinMM()) + "mm";
  }
  if(mode == MODE_MAN){
    output = "Man  ";
    output += motor1.getDir() ? ">" : "<";
    output += " " + String(motor1.getPosinMM()) + "mm";
  }
  if(mode == MODE_SPEED){
    output = "Speed: ";
    output += (String)speedNew;
  }
  if(mode == MODE_DELAY){
    output = "Delay: ";
    output += (String)delayNew;
  }
  return output;
}

String Controller::displayLCD1(){
  String output;
  output = "MODE ";
  if(mode == MODE_AUTO){
    output += motor1.isActive() ? "OFF" : "ON ";
    output += " <   >";
  }
  if(mode == MODE_MAN){
    output += "    <   >";
  }
  if(mode == MODE_SPEED){
    output += speedDirty ? "SET " : "    ";
    output += "-   +";
  }
  if(mode == MODE_DELAY){
    output += delayDirty ? "SET " : "    ";
    output += "-   +";
  }
  
  //output = "test";
  //output = (String)motor1.pulseCount;
  return output;
}

BB420_timer::BB420_timer(){
  delayActive = false;
  delayStart = millis();
  delayTime = 30;   
}

bool BB420_timer::delayOn(){
  bool returnValue = false;
  if(delayActive){
    if( (delayStart+(delayTime*1000)) < millis() ){
      returnValue = false;
      delayActive = false;
    }
    else{
      returnValue = true;
    }
  }
  return returnValue;
}

bool BB420_timer::startDelay(){
  delayActive = true;
  delayStart = millis();
  return true;
}


