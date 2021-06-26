
#include <SPI.h>
#include <LoRa.h>

#include "Controls.h"
#include "Buttons.h"
#include "CommonConstants.h"
#include "Radio.h"
#include "DataManager.h"

// Constants:
const int trimStep = 1; // Degrees to step trim per click

BUTTON_HANDLER ButtonHandler;
DATA_MANAGER TelemetryManager(new char[64], 64);
DATA_MANAGER RadioDataManager(new char[maxRadioMessageLength], maxRadioMessageLength);
RADIO Radio(maxRadioMessageLength);

void setup() {
  analogReference(EXTERNAL); // Connected to the 3.3V supply
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(38400);
  
  RadioDataManager.addData(reg_setDropDoor,lockDoorSignal); // Door is put in locked position initially
  
  // LoRa Setup:
  if (!Radio.begin(433E6,2)){ // Frequency, txPower in dBm (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW)
    Serial.println("LoRa setup failed.");
    while (1); // If LoRa doesn't begin, stop execution.
  }
}

void loop() {
  // Control surface classes, initiated with the ranges of the controls: inputMin,inputMax,servoMin,servoCentre,servoMax,maxTrimDeviation:
  static CONTROL ailerons (potLOW,potHIGH,30,90,150,20);
  static CONTROL elevator (potLOW,potHIGH,30,90,150,20);
  static CONTROL rudder   (potLOW,potHIGH,30,90,150,20);
  static CONTROL throttle (potLOW,potHIGH,0 ,90,180,0 );

  static unsigned long currentTime = 0; // Time since program start
  static unsigned long nextTelemetryTime = 0; // Time at which the next telemetry packet should be sent
  static int avgLoopTime;
  avgLoopTime = (avgLoopTime + (millis()-currentTime)) / 2; // Take rolling average of loop times
  currentTime = millis();

  // Update the servo positions for each control surface:
  ailerons.updateServoPosition(analogRead(AILERON_PIN));
  elevator.updateServoPosition(analogRead(ELEVATOR_PIN));
  rudder.updateServoPosition(analogRead(RUDDER_PIN));
  throttle.updateServoPosition(analogRead(THROTTLE_PIN));
  
  RadioDataManager.addData(reg_setAilerons, ailerons.getPos());
  RadioDataManager.addData(reg_setElevator, elevator.getPos());
  RadioDataManager.addData(reg_setRudder,   rudder.getPos());
  RadioDataManager.addData(reg_setThrottle, throttle.getPos());
  
  // Handle button presses:
  int buttonPressed = ButtonHandler.getButtonPressed(analogRead(BUTTON_PIN));
  unsigned long timePressed = ButtonHandler.getPressedTime();
  
  if( ButtonHandler.pressHandled() == false) {
    bool isHandled = true; // Only set to false if a default section is reached
    
    // Buttons that require a long press (1 second):
    if(timePressed > 750){
      switch(buttonPressed){
        case camera_button:
          RadioDataManager.addData(reg_setDropDoor,unlockDoorSignal); // The servo is only closed again on reset
          TelemetryManager.addData(reg_setDropDoor,unlockDoorSignal);
          break;
        default:
          isHandled = false; // None of these buttons, so it wasn't handled
      }
    }else if(timePressed > 20){ // Buttons that require only a short press (50ms):
      
      switch ( buttonPressed ) {
        case no_button:
          break;
        case left_arrow:
          ailerons.adjustTrim(-trimStep);
          break;
        case right_arrow:
          ailerons.adjustTrim(trimStep);
          break;
        case up_arrow:
          elevator.adjustTrim(trimStep);
          break;
        case down_arrow:
          elevator.adjustTrim(-trimStep);
          break;
        case left_skip:
          rudder.adjustTrim(-trimStep);
          break;
        case right_skip:
          rudder.adjustTrim(trimStep);
          break;
    
        case centre_button:
          ailerons.resetTrim();
          elevator.resetTrim();
          break;
        case star_button:
          rudder.resetTrim();
          break;

        default:
          isHandled = false; // None of these buttons, so it wasn't handled.
      }
    }else{ // Not held for long enough
      isHandled = false;
    }
    if( isHandled == true && buttonPressed != no_button){ // A button has just been pressed
      tone(3, 1024, 100);
    }
    ButtonHandler.updatePressHandled(isHandled); // Pass in whether press was handled
  }
  
  // Receive data from PI
  
  // Transmit data to plane
  unsigned long txStartTime = millis();
  char* outDataBuffer; // Will point to the beginning of the radio data buffer array
  int outDataLength; // Will contain length of the data in the radio data buffe array
  RadioDataManager.getData(&outDataBuffer,&outDataLength);
  Radio.transmitData(outDataBuffer,outDataLength);
  
  // Receive data from plane and pass straight to PI
  static char inDataBuffer[maxRadioMessageLength]; // Actual dataBuffer array
  int inDataLength; // Will contain length of data in the data buffer
  Radio.receiveData(inDataBuffer,&inDataLength);
  TelemetryManager.addDataArray(inDataBuffer,inDataLength);
  
  // Feedback data to PI:
  if(currentTime > nextTelemetryTime){
    nextTelemetryTime = millis() + 250; // 0.25 seconds from now
    
    TelemetryManager.addData(reg_setAilerons, ailerons.getPos());
    TelemetryManager.addData(reg_setElevator, elevator.getPos());
    TelemetryManager.addData(reg_setRudder,   rudder.getPos());
    TelemetryManager.addData(reg_setThrottle, throttle.getPos());
        
    TelemetryManager.addData(reg_aileronTrimPos,ailerons.getTrimCentre());
    TelemetryManager.addData(reg_elevatorTrimPos,elevator.getTrimCentre());
    TelemetryManager.addData(reg_rudderTrimPos,rudder.getTrimCentre());
    
    TelemetryManager.addData(reg_groundLoopSpeed,constrain(avgLoopTime,0,255));

    char* outDataBuffer; // Will point to the beginning of the telemetryBuffer array
    int outDataLength; // Will contain length of the data in the telemetryBuffer array
    TelemetryManager.getData(&outDataBuffer,&outDataLength);
    transmitToPI(outDataBuffer,outDataLength); // After this line, *outDataBuffer and outDataLength go out of scope.
  }
}

void transmitToPI(char txdata[], int len){
  Serial.print((char) len); // Send length of message being sent.
  Serial.write(txdata,len);
}
