
// Pin Numbers:
#define ELEVATOR_PIN A1
#define AILERON_PIN A0
#define THROTTLE_PIN A2
#define RUDDER_PIN A3
#define BUTTON_PIN A4

// analogRead LOW and analogRead HIGH:
#define potLOW 0 // Minimum potentiometer reading is 0.0V
#define potHIGH 676 // Maximum potentiometer analogRead value (for 3.3 volts)

// Button analogRead values (assuming 3.3V is supplied):
#define no_button 695

#define centre_button 352 // Reset elevator/aileron trim
#define left_arrow 308 // Trim ailerons left
#define right_arrow 403 // Trim ailerons right
#define up_arrow 460 // Trim elevator up
#define down_arrow 270 // Trime elevator down

#define left_skip 58 // Trim rudder left
#define right_skip 146 // Trim rudder right
#define camera 224
#define video 189
#define star_button 96 // Reset rudder trim

#define left_button 14
#define right_button 506

#define numButtons 13 // Number of different buttons in buttons[], inclusive of unpressed state
#define tolerance 2 // How close reading must be to actual button value

int buttons[] = {no_button,centre_button,left_arrow,right_arrow,up_arrow,down_arrow,left_skip,right_skip,camera,video,star_button,left_button,right_button};

int getButtonPressed(int pinReading) {
  static int buttonPosition = no_button; // The current button being pressed
  static int prevButtonPosition = no_button; // The button being pressed in the previous loop
  static unsigned long lastTrigger; // The time at which the last button trigger event occurred (i.e. a value was returned)
  
  int pinMin = pinReading - tolerance;
  int pinMax = pinReading + tolerance;

  prevButtonPosition = buttonPosition;
  
  for(int i=0;i<numButtons;i++){
    if(pinMin < buttons[i] && pinMax > buttons[i]){ // If button i is currently held down:
      buttonPosition = buttons[i];
      if(buttons[i] != buttonPosition){ // If a button state has just changed
        lastTrigger = millis();
      }
      break; // Button found, no need to continue
    }
  }

  if(millis()-lastTrigger >= 200){ // If button has been pressed for 200ms or more:
    if(buttonPosition != prevButtonPosition){ // If the button state has just changed:
      return buttonPosition; // Trigger button press event.
    }
  }

  return no_button; // If no event has been triggered, return a no_button
}

