

class CONTROL {
  public:
    int inMIN, inMAX, outMIN, outCENTRE, outMAX; // Control range cannot be changed (outCENTRE refers to the default centre position)
    int trimCENTRE; // The output servo value which becomes the new centrepoint (i.e. the value that is read out when the stick is in the middle)
    int pos; // Current servo position
    
    CONTROL (int _inMIN, int _inMAX, int _outMIN, int _outCENTRE, int _outMAX){
      inMIN = _inMIN;
      inMAX = _inMAX;
      outMIN = _outMIN;
      outCENTRE = _outCENTRE;
      outMAX = _outMAX;
      
      trimCENTRE = outCENTRE; // Trim centre defaults to the actual centre position.
    }

    int updateServoPosition(int newPinReading){
      // Update avgPinReading to the rolling average of it's previous value + incoming reading:
      avgPinReading  = ((avgPinReading  * (8-1)) + newPinReading )  / 8;
      pos = mapControlValue(avgPinReading);
      
      return pos;
    }
  
  private:
    int avgPinReading; // Hold the rolling average analog read values for this pin

    int mapControlValue(int pinReading){
      // Map the input pinReading to an output servo value //
      float fractionalPosition = ((pinReading-inMIN)*1.0) / ((inMAX-inMIN)*1.0); // Input position as a fraction between 0 and 1
      float fractionalCentrePosition = ((trimCENTRE-outMIN)*1.0) / ((outMAX-outMIN)*1.0); // Centre position as a fraction between 0 and 1
    
      if(fractionalPosition <= 0.5){
        // pinReading is below or at the corresponding centrePosition
        int fractionalLowerHalfPosition = (fractionalPosition * 2.0)*1024.0; // Where 0 represents input/output minimum, and 1024 represents the input position corresponding to output centre
        int outPosition = map(fractionalLowerHalfPosition,0,1024,outMIN,trimCENTRE);
        return constrain(outPosition, outMIN, outMAX);
      }else{
        // Pin reading is above the correseponding centre position
        int fractionalUpperHalfPosition = ((1.0-fractionalPosition) * 2.0)*1024.0; // Where 1024 represents input/output centre, and 0 represents the input position corresponding to output max
        int outPosition = map(fractionalUpperHalfPosition,1024,0,trimCENTRE,outMAX);
        return constrain(outPosition, outMIN, outMAX);
      }
    }
};


////////// Button analogRead values (assuming 3.3V is supplied)
#define noButton 695

#define centreButton 352
#define left_arrow 308
#define right_arrow 403
#define up_arrow 460
#define down_arrow 270

#define left_skip 58
#define right_skip 146
#define camera 224
#define video 189
#define star_button 96

#define left_button 14
#define right_button 506

#define num_buttons 13 // Number of different buttons in buttons[], inclusive of unpressed state
#define tolerance 4 // How close reading must be to actual button value

int buttons[] = {noButton,centreButton,left_arrow,right_arrow,up_arrow,down_arrow,left_skip,right_skip,camera,video,star_button,left_button,right_button};

int getButtonPressed(int pinReading) {
  static int buttonPressed = 0; // The current button being pressed
  static int prevButtonPressed = 0; // The previous button being pressed
  
  int pinMin = pinReading - tolerance;
  int pinMax = pinReading + tolerance;
  
  for(int i=0;i<num_buttons;i++){
    if(pinMin < buttons[i] && pinMax > buttons[i]){ // If button i is currently pressed:
      if(prevButtonPressed == i){ // If button i was also held down last time:
        buttonPressed = i; // Change buttonPressed to the current button
      }
      prevButtonPressed = i;
      break;
    }
  }

  return buttonPressed;
}


