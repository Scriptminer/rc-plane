
// Pin Numbers:
#define ELEVATOR_PIN A0
#define AILERON_PIN A1
#define THROTTLE_PIN A2
#define RUDDER_PIN A3
#define BUTTON_PIN A4

// analogRead LOW and analogRead HIGH:
#define potLOW 0 // Minimum potentiometer reading is 0.0V
#define potHIGH 1023 // Maximum potentiometer analogRead value (analogReference is tied to the 5.0V supply which supplies the potentiometers)

// Button analogRead values (assuming 5.0V is supplied):
#define no_button 1022

#define centre_button 515 // Reset elevator/aileron trim
#define left_arrow 449 // Trim ailerons left
#define right_arrow 590 // Trim ailerons right
#define up_arrow 674 // Trim elevator up
#define down_arrow 392 // Trime elevator down

#define left_skip 79 // Trim rudder left
#define right_skip 209 // Trim rudder right
#define camera_button 325 // Trigger drop door open
#define video_button 272
#define star_button 136 // Reset rudder trim

#define left_button 14
#define right_button 742

#define numButtons 13 // Number of different buttons in buttons[], inclusive of unpressed state
#define tolerance 5 // How close reading must be to actual button value

class ButtonHandler {
  public:
    
    int getButtonPressed(int pinReading) {
      // Update button press information, return the current button being held down.
      int pinMin = pinReading - tolerance;
      int pinMax = pinReading + tolerance;
      
      for(int i=0;i<numButtons;i++){
        if(pinMin < buttons[i] && pinMax > buttons[i]){ // If button i is currently held down:
          if(buttonPosition != buttons[i]){ // The button being pressed has just changed
            lastPressTime = millis(); // Updates timestamp, given this is a new press event.
            thisPressHandled = false; // This is a new press event, so it has not been handled yet.
            buttonPosition = buttons[i]; // Updates to the new button position
          }
          return buttonPosition; // Button found, no need to continue
        }
      }
      
      return no_button; // If no button found, return a no_button
    }

    unsigned long getPressedTime() {
      // Returns how long the current button has been pressed for.
      return (millis() - lastPressTime);
    }

    void updatePressHandled() {
      // Internally recognises that this current button press has been handled.
      thisPressHandled = true;
    }

    bool isPressHandled() {
      // Return whether the current button press has been handled or not.
      return thisPressHandled;
    }

  private:
    int buttonPosition = no_button; // The current button being pressed
    unsigned long lastPressTime = 0; // The time at which the last button trigger event occurred (i.e. a value was returned)
    bool thisPressHandled = false; // If an event has been triggered for the current button press yet
    
    int buttons[numButtons] = {no_button,centre_button,left_arrow,right_arrow,up_arrow,down_arrow,left_skip,right_skip,camera_button,video_button,star_button,left_button,right_button};
};

