
////////// Pin Numbers //////////
#define ELEVATOR_PIN A1
#define AILERON_PIN A0
#define THROTTLE_PIN A2
#define RUDDER_PIN A3
#define BUTTON_PIN A4
//////////

#define potLOW 0 // Minimum potentiometer reading is 0.0V
#define potHIGH 676 // Maximum potentiometer analogRead value (for 3.3 volts)

struct RANGE {
  int inMIN;
  int inMAX;
  int outMIN;
  int outCENTRE;
  int defaultOutCENTRE;
  int outMAX;
};

#include "Controls.h"

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  static int aileronPinReading, elevatorPinReading, throttlePinReading, rudderPinReading = 0; // Will hold the (averaged) analog read values for each pin
  static int aileronPos, elevatorPos, throttlePos, rudderPos = 0; // Current positions to be sent to servos. 
  // Current servo control ranges, in the form [inputMin,inputMax,servoMin,servoCentre,servoMax]. Only servoCentre is be changed dynamically.
  static RANGE aileronRange  = {potLOW,potHIGH,30,90,90,150};
  static RANGE elevatorRange = {potLOW,potHIGH,30,90,90,150};
  static RANGE rudderRange   = {potLOW,potHIGH,30,90,90,150};
  static RANGE throttleRange = {potLOW,potHIGH,00,90,90,180};
  
  // Update pin readings for each control surface to the rolling average of their previous value + current reading:
  aileronPinReading  = ((aileronPinReading  * (16-1)) + analogRead(AILERON_PIN) )  / 16;
  elevatorPinReading = ((elevatorPinReading * (16-1)) + analogRead(ELEVATOR_PIN))  / 16;
  throttlePinReading = ((throttlePinReading * (16-1)) + analogRead(THROTTLE_PIN))  / 16;
  rudderPinReading   = ((rudderPinReading   * (16-1)) + analogRead(RUDDER_PIN)  )  / 16;
  
  // Update servo positions:
  aileronPos  = mapControlValue(aileronPinReading,aileronRange);
  elevatorPos = mapControlValue(elevatorPinReading,elevatorRange);
  rudderPos   = mapControlValue(rudderPinReading,rudderRange);
  throttlePos = mapControlValue(throttlePinReading,throttleRange);

  // Print servo positions:
  Serial.print("0,180,");
  Serial.print(aileronPos);Serial.print(",");
  Serial.print(elevatorPos);Serial.print(",");
  Serial.print(rudderPos);Serial.print(",");
  Serial.print(throttlePos);Serial.println();
}


