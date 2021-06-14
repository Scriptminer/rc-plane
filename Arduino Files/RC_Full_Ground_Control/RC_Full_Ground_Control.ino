
#include <SPI.h>
#include <LoRa.h>

#include "Controls.h"
#include "Buttons.h"

// Constants:
const int trimStep = 1; // Degrees to step trim per click

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200);


  // Begin LoRa:
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1); // If LoRa doesn't begin, stop execution.
  }
  
  LoRa.setTxPower(2); // 10dBm, which is 10mW (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW)

}

void loop() {
  // Control surface classes, initiated with the ranges of the controls: inputMin,inputMax,servoMin,servoCentre,servoMax,maxTrimDeviation:
  static CONTROL ailerons (potLOW,potHIGH,30,90,150,20);
  static CONTROL elevator (potLOW,potHIGH,30,90,150,20);
  static CONTROL rudder   (potLOW,potHIGH,30,90,150,20);
  static CONTROL throttle (potLOW,potHIGH,0 ,90,180,0 );
  
  // Update the servo positions for each control surface:
  ailerons.updateServoPosition(analogRead(AILERON_PIN));
  elevator.updateServoPosition(analogRead(ELEVATOR_PIN));
  rudder.updateServoPosition(analogRead(RUDDER_PIN));
  throttle.updateServoPosition(analogRead(THROTTLE_PIN));
  
  // Handle button presses:
  switch ( getButtonPressed(analogRead(BUTTON_PIN)) ){
    case 0:
      break;
    case left_arrow:
      ailerons.adjustTrim(trimStep);
      break;
    case right_arrow:
      ailerons.adjustTrim(-trimStep);
      break;
    case up_arrow:
      elevator.adjustTrim(trimStep);
      break;
    case down_arrow:
      elevator.adjustTrim(-trimStep);
      break;
    case left_skip:
      rudder.adjustTrim(trimStep);
      break;
    case right_skip:
      rudder.adjustTrim(-trimStep);
      break;

    case centre_button:
      ailerons.resetTrim();
      elevator.resetTrim();
      break;
    case star_button:
      rudder.resetTrim();
      break;
  }

  // Print servo positions:
  /*Serial.print("0,180,");
  Serial.print(analogRead(A5));Serial.print(",");
  Serial.print(ailerons.pos);Serial.print(",");
  Serial.print(elevator.pos);Serial.print(",");
  Serial.print(rudder.pos);Serial.print(",");
  Serial.print(throttle.pos);Serial.println();*/

  // Receive data from PI
  
  // Transmit data to plane
  char data[8] = {0,ailerons.pos,1,elevator.pos,2,rudder.pos,3,throttle.pos};
  transmitToPlane(data,8);
  
  // Receive data from plane
  
  // Feedback data to PI: (remember to include status data for UNO e.g. loops per second)
  
  

}

void transmitToPlane(char txdata[], int len){
  LoRa.beginPacket();
  for(int i=0;i<len;i++){
    LoRa.print(txdata[i]);
  }
  LoRa.endPacket();
}

