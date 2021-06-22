
// Pin Numbers:
#define ELEVATOR_PIN A0
#define AILERON_PIN A1
#define THROTTLE_PIN A2
#define RUDDER_PIN A3
#define BUTTON_PIN A4

// analogRead LOW and analogRead HIGH:
#define potLOW 0 // Minimum potentiometer reading is 0.0V
#define potHIGH 1023 // Maximum potentiometer analogRead value (analogReference is tied to the 3.3V supply which supplies the potentiometers)

// Button analogRead values (assuming 3.3V is supplied):
#define no_button 1023

#define centre_button 530 // Reset elevator/aileron trim
#define left_arrow 463 // Trim ailerons left
#define right_arrow 606 // Trim ailerons right
#define up_arrow 691 // Trim elevator up
#define down_arrow 406 // Trime elevator down

#define left_skip 88 // Trim rudder left
#define right_skip 221 // Trim rudder right
#define camera_button 338
#define video_button 284
#define star_button 146 // Reset rudder trim

#define left_button 22
#define right_button 760

#define numButtons 13 // Number of different buttons in buttons[], inclusive of unpressed state
#define tolerance 15 // How close reading must be to actual button value

class BUTTON_HANDLER {
  public:
    
    int getButtonPressed(int pinReading) {
      int pinMin = pinReading - tolerance;
      int pinMax = pinReading + tolerance;
      
      for(int i=0;i<numButtons;i++){
        if(pinMin < buttons[i] && pinMax > buttons[i]){ // If button i is currently held down:
          if(buttonPosition != buttons[i]){ // Button pressed has just changed
            lastPressTime = millis();
            thisPressHandled = false;
          }
          buttonPosition = buttons[i];
          return buttonPosition; // Button found, no need to continue
        }
      }
    
      return no_button; // If no button found, return a no_button
    }

    unsigned long getPressedTime() {
      // Returns how long the current button has been pressed for
      return (millis() - lastPressTime);
    }

    void updatePressHandled() {
      thisPressHandled = true;
    }

    bool pressHandled() {
      // Return whether the current button press has been handled or not
      return thisPressHandled;
    }

  private:
    int buttonPosition = no_button; // The current button being pressed
    unsigned long lastPressTime = 0; // The time at which the last button trigger event occurred (i.e. a value was returned)
    bool thisPressHandled = false; // If an event has been triggered for the current button press yet
    
    int buttons[numButtons] = {no_button,centre_button,left_arrow,right_arrow,up_arrow,down_arrow,left_skip,right_skip,camera_button,video_button,star_button,left_button,right_button};
};

