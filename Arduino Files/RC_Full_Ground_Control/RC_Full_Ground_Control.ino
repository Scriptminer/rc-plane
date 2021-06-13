
////////// Pin Numbers //////////
#define ELEVATOR_PIN A1
#define AILERON_PIN A0
#define THROTTLE_PIN A2
#define RUDDER_PIN A3
#define BUTTON_PIN A4
//////////

#define potLOW 0 // Minimum potentiometer reading is 0.0V
#define potHIGH 676 // Maximum potentiometer analogRead value (for 3.3 volts)

#include "Controls.h"

void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  // Control surface classes, initiated with the ranges of the controls: inputMin,inputMax,servoMin,servoCentre,servoMax:
  static CONTROL ailerons (potLOW,potHIGH,30,90,150);
  static CONTROL elevator (potLOW,potHIGH,30,90,150);
  static CONTROL rudder   (potLOW,potHIGH,30,90,150);
  static CONTROL throttle (potLOW,potHIGH,00,90,180);

  // Update the servo positions for each control surface:
  ailerons.updateServoPosition(analogRead(AILERON_PIN));
  elevator.updateServoPosition(analogRead(ELEVATOR_PIN));
  rudder.updateServoPosition(analogRead(RUDDER_PIN));
  throttle.updateServoPosition(analogRead(THROTTLE_PIN));

  // Print servo positions:
  Serial.print("0,180,");
  Serial.print(ailerons.pos);Serial.print(",");
  Serial.print(elevator.pos);Serial.print(",");
  Serial.print(rudder.pos);Serial.print(",");
  Serial.print(throttle.pos);Serial.println();
}


