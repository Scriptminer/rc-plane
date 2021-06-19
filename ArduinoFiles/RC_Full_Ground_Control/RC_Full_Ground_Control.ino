
#include <SPI.h>
#include <LoRa.h>

#include "Controls.h"
#include "Buttons.h"
#include "CommonConstants.h"

// Constants:
const int trimStep = 1; // Degrees to step trim per click

void setup() {
  // put your setup code here, to run once:
  analogReference(EXTERNAL); // Connected to the 3.3V supply
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200);


  // Begin LoRa:
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1); // If LoRa doesn't begin, stop execution.
  }
  
  LoRa.setTxPower(2); // 10dBm, which is 10mW (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW)

}

BUTTON_HANDLER ButtonHandler;

void loop() {
  // Control surface classes, initiated with the ranges of the controls: inputMin,inputMax,servoMin,servoCentre,servoMax,maxTrimDeviation:
  static CONTROL ailerons (potLOW,potHIGH,30,90,150,20);
  static CONTROL elevator (potLOW,potHIGH,30,90,150,20);
  static CONTROL rudder   (potLOW,potHIGH,30,90,150,20);
  static CONTROL throttle (potLOW,potHIGH,0 ,90,180,0 );

  byte radioTransmitBuffer[maxRadioMessageLength];
  int radioTransmitBufferPos = 0;
  
  // Update the servo positions for each control surface:
  ailerons.updateServoPosition(analogRead(AILERON_PIN));
  elevator.updateServoPosition(analogRead(ELEVATOR_PIN));
  rudder.updateServoPosition(analogRead(RUDDER_PIN));
  throttle.updateServoPosition(analogRead(THROTTLE_PIN));

  radioTransmitBuffer[0] = reg_setAilerons; radioTransmitBuffer[1] = ailerons.getPos(),
  radioTransmitBuffer[2] = reg_setElevator; radioTransmitBuffer[3] = elevator.getPos(),
  radioTransmitBuffer[4] = reg_setRudder;   radioTransmitBuffer[5] = rudder.getPos(),
  radioTransmitBuffer[6] = reg_setThrottle; radioTransmitBuffer[7] = throttle.getPos(),
  
  radioTransmitBufferPos = 8;
  
  //Serial.println(analogRead(BUTTON_PIN));
  
  // Handle button presses:
  int buttonPressed = ButtonHandler.getButtonPressed(analogRead(BUTTON_PIN));
  unsigned long timePressed = ButtonHandler.getPressedTime();
  if( ButtonHandler.pressHandled() == false) {
    // Buttons that require only a short press (50ms):
    if(timePressed > 50){
      switch ( buttonPressed ) {
        case no_button:
          ButtonHandler.updatePressHandled();
          break;
        case left_arrow:
          ailerons.adjustTrim(-trimStep);
          ButtonHandler.updatePressHandled();
          break;
        case right_arrow:
          ailerons.adjustTrim(trimStep);
          ButtonHandler.updatePressHandled();
          break;
        case up_arrow:
          elevator.adjustTrim(-trimStep);
          ButtonHandler.updatePressHandled();
          break;
        case down_arrow:
          elevator.adjustTrim(trimStep);
          ButtonHandler.updatePressHandled();
          break;
        case left_skip:
          rudder.adjustTrim(-trimStep);
          ButtonHandler.updatePressHandled();
          break;
        case right_skip:
          rudder.adjustTrim(trimStep);
          ButtonHandler.updatePressHandled();
          break;
    
        case centre_button:
          ailerons.resetTrim();
          elevator.resetTrim();
          ButtonHandler.updatePressHandled();
          break;
        case star_button:
          rudder.resetTrim();
          ButtonHandler.updatePressHandled();
          break;
      }
    }

    // Buttons that require a long press (1 second):
    if(timePressed > 1000){
      switch(buttonPressed){
        case camera_button:
          radioTransmitBuffer[radioTransmitBufferPos] = reg_setDropDoor;
          radioTransmitBuffer[radioTransmitBufferPos+1] = unlockDoorSignal; // There isn't currently any provision for returning the servo to the closed position, as this with the current setup is unnecessary.
          radioTransmitBufferPos += 2;
          ButtonHandler.updatePressHandled();
          break;
      }
    }
  }

  // Print servo positions:
  /*Serial.print("0,180,");
  Serial.print(ailerons.pos);Serial.print(",");
  Serial.print(elevator.pos);Serial.print(",");
  Serial.print(rudder.pos);Serial.print(",");
  Serial.print(throttle.pos);Serial.println();*/
  // Receive data from PI
  
  // Transmit data to plane

  
  transmitToPlane(radioTransmitBuffer,radioTransmitBufferPos);
  //Serial.println(ButtonHandler.getPressedTime());
  
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

