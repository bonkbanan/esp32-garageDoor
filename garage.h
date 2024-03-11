#pragma once

/*
enum TargetDoorStates {
  opened, 0
  closed  1
};

enum DoorStates {
  opened,  0
  closed,  1
  opening, 2
  closing, 3
  
  stopped  4 - not moving
};
*/

struct GarageDoorService : Service::GarageDoorOpener {
  int dooropenpin;
  int sensorpin;
  int activebutton = 0;
  unsigned long openTime;
  unsigned long closeTime;
  unsigned long lastMs = millis();

  SpanCharacteristic *currentDoorState; // door state
  SpanCharacteristic *targetDoorState;
  SpanCharacteristic *obstructionDetected; // bool

  GarageDoorService(int dooropenpin, int sensorpin, unsigned long openTime, unsigned long closeTime) : Service::GarageDoorOpener(){
    this->dooropenpin = dooropenpin;
    this->sensorpin = sensorpin;
    this->openTime = openTime;
    this->closeTime = closeTime;
    pinMode(dooropenpin, OUTPUT);
    pinMode(sensorpin,INPUT_PULLUP);

    this->currentDoorState = new Characteristic::CurrentDoorState();
    this->targetDoorState = new Characteristic::TargetDoorState();
    this->obstructionDetected = new Characteristic::ObstructionDetected();
    this->obstructionDetected->setVal(false);
  }

  boolean update() {
    if(targetDoorState->getVal() != targetDoorState->getNewVal()) { // if value change
      if(targetDoorState->getNewVal() == 0) { // OPEN the door
        if(currentDoorState->getVal() == 1) { // if already closed then open
          currentDoorState->setVal(2);
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          lastMs = millis();
          activebutton = 1;
          return true;
        } else if(currentDoorState->getVal() == 3) {// if currently closing then cancel close and open
          currentDoorState->setVal(2);
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          delay(1000);
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          activebutton = 1;

          float closeDoorPercent = (float)(millis()-lastMs)/(float)(closeTime);
          lastMs = (unsigned long)(millis()+(float)(openTime)*closeDoorPercent); // currentTime + ( openingTime same percent as closed )
          return true;
        }
      } else if(targetDoorState->getNewVal() == 1) { // CLOSE the door
        if(currentDoorState->getVal() == 0) { // if opened then close
          currentDoorState->setVal(3);
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          lastMs = millis();
          activebutton = 1;

          return true;
        }
        if(currentDoorState->getVal() == 2) {// if currently opening then cancel open and close
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          delay(1000);
          digitalWrite(dooropenpin, HIGH);
          delay(200);
          digitalWrite(dooropenpin, LOW);
          currentDoorState->setVal(3);
          activebutton = 1;

          float openDoorPercent = (float)(millis()-lastMs)/(float)(openTime);
          lastMs = (unsigned long)(millis()+(float)(closeTime)*openDoorPercent); // currentTime + ( closeTime same percent as closed )
          return true;
        }
      }
    }
  }

  void loop() {
   

    
    unsigned long timeElapsed = millis()-lastMs;

    if(currentDoorState->getVal() == 3 && activebutton == 1) { // if closing
      if(timeElapsed >= closeTime) { // if close time elapsed
        currentDoorState->setVal(1); // set closed
        activebutton = 0;
      }
    } else if(currentDoorState->getVal() == 2 && activebutton == 1) { // if opening
      if(timeElapsed >= openTime) { // if open time elapsed
        currentDoorState->setVal(0); // set opened
        activebutton = 0;
      }
    }

    if(digitalRead(sensorpin) == HIGH && (targetDoorState->getVal() == targetDoorState->getNewVal() 
      && targetDoorState->getVal() == 1) && currentDoorState->getVal() != 0 && activebutton == 0){
      currentDoorState->setVal(0);
      targetDoorState->setVal(0);
    }
    if(digitalRead(sensorpin) == LOW && (targetDoorState->getVal() == targetDoorState->getNewVal() 
      && targetDoorState->getVal() == 0) && currentDoorState->getVal() != 1 && activebutton == 0){
      currentDoorState->setVal(1);
      targetDoorState->setVal(1);
  }
  }
};