int mapControlValue(int pinReading,RANGE controlRange){
  // Map the input pinReading to an output servo value
  float fractionalPosition = ((pinReading-controlRange.inMIN)*1.0) / ((controlRange.inMAX-controlRange.inMIN)*1.0); // Input position as a fraction between 0 and 1
  float fractionalCentrePosition = ((controlRange.outCENTRE-controlRange.outMIN)*1.0) / ((controlRange.outMAX-controlRange.outMIN)*1.0); // Centre position as a fraction between 0 and 1
  /*Serial.println("\n\n");
  Serial.println("Attempting mapping process:");
  Serial.print("pinReading: "); Serial.println(pinReading);
  Serial.print("inMIN: "); Serial.println(controlRange.inMIN);
  Serial.print("inMAX: "); Serial.println(controlRange.inMAX);
  Serial.print("outMIN: "); Serial.println(controlRange.outMIN);
  Serial.print("outCENTRE: "); Serial.println(controlRange.outCENTRE);
  Serial.print("outMAX: "); Serial.println(controlRange.outMAX);
  Serial.println("---");
  Serial.print("fractionalPosition: "); Serial.println(fractionalPosition);
  Serial.print("fractionalCentrePosition: "); Serial.println(fractionalCentrePosition);
  Serial.print("---");*/
  if(fractionalPosition <= 0.5){ // pinReading is below or at the corresponding centrePosition
    int fractionalLowerHalfPosition = (fractionalPosition * 2.0)*1024.0; // Where 0 represents input/output minimum, and 1024 represents the input position corresponding to output centre
    //Serial.print("fractionalLowerHalfPosition: "); Serial.println(fractionalLowerHalfPosition);
    int outPosition = map(fractionalLowerHalfPosition,0,1024,controlRange.outMIN,controlRange.outCENTRE);
    //Serial.print("outPosition: "); Serial.println(outPosition);
    return constrain(outPosition, controlRange.outMIN, controlRange.outMAX);
  }else{ // Pin reading is above the correseponding centre position
    int fractionalUpperHalfPosition = ((1.0-fractionalPosition) * 2.0)*1024.0; // Where 1024 represents input/output centre, and 0 represents the input position corresponding to output max
    //Serial.print("fractionalUpperHalfPosition: "); Serial.println(fractionalUpperHalfPosition);
    int outPosition = map(fractionalUpperHalfPosition,1024,0,controlRange.outCENTRE,controlRange.outMAX);
    //Serial.print("outPosition: "); Serial.println(outPosition);
    return constrain(outPosition, controlRange.outMIN, controlRange.outMAX);
  }
  
}

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


