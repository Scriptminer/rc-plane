
#include "Controls.h"
#include "Buttons.h"

// Constants:
const int trimStep = 1; // Degrees to step trim per click

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200);
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
  Serial.print("0,180,");
  Serial.print(ailerons.pos);Serial.print(",");
  Serial.print(elevator.pos);Serial.print(",");
  Serial.print(rudder.pos);Serial.print(",");
  Serial.print(throttle.pos);Serial.println();
}
