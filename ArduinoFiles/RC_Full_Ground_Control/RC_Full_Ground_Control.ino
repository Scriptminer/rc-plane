
//#define __HUMAN_READABLE_SERIAL__

#include <SPI.h>
#include <LoRa.h>

#include "Controls.h"
#include "Buttons.h"
#include "CommonConstants.h"
#include "Radio.h"
#include "DataManager.h"

// Constants:
const int trimStep = 1; // Degrees to step trim per click

ButtonHandler buttonHandler;
DataManager telemetryManager(new char[64], 64);
DataManager radioDataManager(new char[maxRadioMessageLength], maxRadioMessageLength);
Radio radio(maxRadioMessageLength, airToGroundFrequency, groundToAirFrequency, airToGroundBandwidth, groundToAirBandwidth, groundToAirTxPower); // maxMessageLength, rxFrequency, txFrequency, rxBandwidth, txBandwidth, txPower in dBm

void setup() {
  analogReference(EXTERNAL); // Connected to the 3.3V supply
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200,SERIAL_8E2);
  
  radioDataManager.addData(reg_setDropDoor,lockDoorSignal); // Door is put in locked position initially
  
  // LoRa Setup:
  if (!radio.begin()){ 
    char failureMessage[] = {reg_groundRadioStarted, 1}; // 1 for startup failure
    transmitToPI(failureMessage,2);
    while (1); // If LoRa doesn't begin, stop execution.
  }
  char successMessage[] = {reg_groundRadioStarted, 0};// 0 for success
  transmitToPI(successMessage,2);
}

void loop() {
  // Control surface classes, initiated with the ranges of the controls: inputMin,inputMax,servoMin,servoCentre,servoMax,maxTrimDeviation:
  static Control ailerons (potLOW,potHIGH,30,90,150,40);
  static Control elevator (potLOW,potHIGH,35,90,180,40);
  static Control rudder   (potLOW,potHIGH,30,90,150,20);
  static Control throttle (potLOW,potHIGH,0 ,90,180,0 );

  static unsigned long currentTime = 0; // Time since program start
  static unsigned long nextGroundTelemetryTransmit = 0; // Time at which the next telemetry packet should be sent.
  static unsigned long nextAirTelemetryRequest = 0; // Time at which the next attempt to request telemetry from the plane should be made.
  static int radioPacketsSentSinceLastTelemetry = 0; // Radio packets sent since the last transmit to the PI
  
  static int avgLoopTime;
  avgLoopTime = (avgLoopTime + (millis()-currentTime)) / 2; // Take rolling average of loop times
  currentTime = millis();
  
  // Update the servo positions for each control surface:
  ailerons.updateServoPosition(analogRead(AILERON_PIN));
  elevator.updateServoPosition(analogRead(ELEVATOR_PIN));
  rudder.updateServoPosition(analogRead(RUDDER_PIN));
  throttle.updateServoPosition(analogRead(THROTTLE_PIN));
  
  radioDataManager.addData(reg_setAilerons, ailerons.getPos());
  radioDataManager.addData(reg_setElevator, elevator.getPos());
  radioDataManager.addData(reg_setRudder,   rudder.getPos());
  radioDataManager.addData(reg_setThrottle, throttle.getPos());
  
  // Handle button presses:
  int buttonPressed = buttonHandler.getButtonPressed(analogRead(BUTTON_PIN));
  unsigned long timePressed = buttonHandler.getPressedTime();
  
  if( buttonHandler.isPressHandled() == false) {
    bool isHandled = true; // Only set to false if a default section is reached
    
    // Buttons that require a long press (0.75 seconds):
    if(timePressed > 750){
      switch(buttonPressed){
        case camera_button:
          radioDataManager.addData(reg_setDropDoor,unlockDoorSignal); // The servo is only closed again on reset
          telemetryManager.addData(reg_setDropDoor,unlockDoorSignal);
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
    if( isHandled == true ){
      buttonHandler.updatePressHandled(); // Sets press handled to true (it is reset to false only on the next button press)
    }
    
  }
  
  // Receive data from PI:
  bool requestingTelemetry = false;

  // Append telemetry request to transmit buffer if it is time to request telemetry:
  if(currentTime > nextAirTelemetryRequest){
    nextAirTelemetryRequest = millis() + airTelemetryInterval;
    requestingTelemetry = true;
    radioDataManager.addData(reg_requestTelemetry, reg_requestTelemetry);
  }
  
  // Transmit data to plane:
  char* outDataBuffer; // Will point to the beginning of the radio data buffer array
  int outDataLength = 0; // Will contain length of the data in the radio data buffer array
  radioDataManager.getData(&outDataBuffer,&outDataLength);
  radioDataManager.resetDataBuffer();
  
  radio.transmitData(outDataBuffer,outDataLength);
  if(outDataLength > 0){
    radioPacketsSentSinceLastTelemetry++;
  }
  
  // Listen for returned telemetry
  if(requestingTelemetry){
    // Receive data from plane and add to buffer for transmit to PI
    static char inDataBuffer[maxRadioMessageLength]; // Actual dataBuffer array
    int inDataLength = 0; // Will contain length of data in the data buffer

    radio.receiveData(inDataBuffer,&inDataLength,millis()+45); // Wait no more than 45ms (avg ideal return seems to be 39-40ms)

    if(inDataLength > 0){
      //telemetryManager.addData(reg_testChannel1, inDataLength);
    }
    telemetryManager.addDataArray(inDataBuffer,inDataLength);
    
  }
  
  // Feedback data to PI:
  if(currentTime > nextGroundTelemetryTransmit){
    
    telemetryManager.addData(reg_setAilerons, ailerons.getPos());
    telemetryManager.addData(reg_setElevator, elevator.getPos());
    telemetryManager.addData(reg_setRudder,   rudder.getPos());
    telemetryManager.addData(reg_setThrottle, throttle.getPos());
        
    telemetryManager.addData(reg_aileronTrimPos,ailerons.getTrimCentre());
    telemetryManager.addData(reg_elevatorTrimPos,elevator.getTrimCentre());
    telemetryManager.addData(reg_rudderTrimPos,rudder.getTrimCentre());
    
    telemetryManager.addData(reg_groundLoopSpeed,avgLoopTime);
    telemetryManager.addData(reg_groundRSSI,radio.getAvgRSSI());
    
    float secondsSinceTelemetry = (millis() - (nextGroundTelemetryTransmit-groundTelemetryInterval)) / 1000.0;
    float radioPacketsPerSecond = ((float) radioPacketsSentSinceLastTelemetry) / secondsSinceTelemetry;
    telemetryManager.addData(reg_groundPacketSendRate, (int) radioPacketsPerSecond);
    radioPacketsSentSinceLastTelemetry = 0; // Reset radio packets counter.
    
    char* outDataBuffer; // Will point to the beginning of the telemetryBuffer array
    int outDataLength = 0; // Will contain length of the data in the telemetryBuffer array
    telemetryManager.getData(&outDataBuffer,&outDataLength);
    telemetryManager.resetDataBuffer();
    
    transmitToPI(outDataBuffer,outDataLength); // After this line, *outDataBuffer and outDataLength go out of scope.
    nextGroundTelemetryTransmit = millis() + groundTelemetryInterval;
  }
}

const char EOL[] = {255,254};
const int EOL_LEN = 2;

void transmitToPI(char txdata[], int len){
  #ifdef __HUMAN_READABLE_SERIAL__
    for(int i=0;i<len;i++){
      Serial.print((int) ((unsigned char) txdata[i]));Serial.print(",");
    }
    Serial.println();
  #else
    for(int i=0; i<len; i++){
      Serial.write(txdata[i]);
      delayMicroseconds(500);
    }
    Serial.write(EOL[0]);
    delayMicroseconds(500);
    Serial.write(EOL[1]);
//    Serial.write(txdata,len);
//    Serial.write(EOL,EOL_LEN); // EOL
  #endif
}

