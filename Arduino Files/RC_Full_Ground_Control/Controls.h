

class CONTROL {
  public:
    int inMIN, inMAX, outMIN, outCENTRE, outMAX; // Control range cannot be changed (outCENTRE refers to the default centre position)
    int maxTrimDeviation; // How far from the default centre can the trim be
    int trimCENTRE; // The output servo value which becomes the new centrepoint (i.e. the value that is read out when the stick is in the middle)
    int pos; // Current servo position
    
    CONTROL (int _inMIN, int _inMAX, int _outMIN, int _outCENTRE, int _outMAX, int _maxTrimDeviation){
      inMIN = _inMIN;
      inMAX = _inMAX;
      outMIN = _outMIN;
      outCENTRE = _outCENTRE;
      outMAX = _outMAX;
      maxTrimDeviation = _maxTrimDeviation;
      
      trimCENTRE = outCENTRE; // Trim centre defaults to the actual centre position.
    }

    int updateServoPosition(int newPinReading){
      // Update avgPinReading to the rolling average of it's previous value + incoming reading:
      avgPinReading  = ((avgPinReading  * (4-1)) + newPinReading )  / 4;
      pos = mapControlValue(avgPinReading);
      
      return pos;
    }

    void adjustTrim(int d){
      trimCENTRE = constrain(trimCENTRE + d, outCENTRE-maxTrimDeviation, outCENTRE+maxTrimDeviation);
    }

    void resetTrim(){
      trimCENTRE = outCENTRE;
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


