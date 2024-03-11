#include <HomeSpan.h>
#include "garage.h"

#define SENSOR 27
#define RELAY 14


void setup() {
  Serial.begin(115200);
  homeSpan.enableOTA();
  homeSpan.begin(Category::GarageDoorOpeners,"Left Garage Door","LeftG");
  
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
    new GarageDoorService(RELAY,SENSOR, 16100, 15410);

}


void loop() {
  homeSpan.poll();
}