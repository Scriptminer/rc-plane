
#define lockedPos 90;
#define unlockedPos 5;

class FLIGHT_DATA {
  public:
    RADIO* Radio; // The plane's onboard radio class
    SENSOR_MANAGER* SensorManager; // The plane's sensor manager
    DATA_MANAGER* TelemetryManager; // The class which manages the telemetry buffer
    
    FLIGHT_DATA (RADIO* planeRadio, SENSOR_MANAGER* planeSensors, DATA_MANAGER* planeTelemetry) : Radio(planeRadio), SensorManager(planeSensors), TelemetryManager(planeTelemetry) {
    }

    bool beginRadio(unsigned long frequency, int txPower){
      return Radio->begin(frequency,txPower);
    }
    
    void updateFlightData(){
      unsigned long timeDif = millis() - flightTime;
      flightTime = millis();
      avgLoopTime = (avgLoopTime + timeDif) / 2; // Take rolling average of loop times
    }

    void updateControls(int inControlState){
      switch(inControlState){
        case 0: // Manual flight:
          break; // do nothing
        case 1: // Emergency:
          throttlePos = 0;   // Shuts off the motor
          elevatorPos = 100; // Tilts the plane slightly up to lose speed and glide into a landing
          aileronPos = 100; // Banks slightly
          rudderPos = 90; // Stays central
        case 2: // Autopilot ???
          break; // Maybe one day will call the autopilot class
      }
    }

    int updateControlState(unsigned long lastSignal){
      // May have more complex criterea in future - just now emergency mode engages after 2 seconds of no signal
      if(flightTime-Radio->getLastSignal() > 2000){ // If last signal was more than 2 seconds ago
        controlState = 1; // Signal Loss: Emergency
      }else{
        controlState = 0; // Manual
      }
      return controlState;
    }

    bool timeForTelemetry(unsigned long telemetryInterval){
      if( flightTime > nextTelemetryTime ){
        nextTelemetryTime = flightTime + 500; // 0.5s from now
        return true;
      }
      else{
        return false;
      }
    }

    bool handleIncomingData(byte inBytes[], int inLength){
      if((inLength%2)!=0){ // If array is not a multiple of 2, an error has occurred
        return false; // Failure
      }

      int messageLength = inLength/2; // Number of commands from ground
      
      for(int i=0;i<messageLength;i++){ // Cycles through all incoming registers
         int inRegister = (unsigned int) inBytes[i*2];
         int inValue = (unsigned int) inBytes[(i*2)+1];
         
         switch(inRegister){
          case reg_setAilerons:
            aileronPos = inValue;
            break;
            
          case reg_setElevator:
            elevatorPos = inValue;
            break;
            
          case reg_setRudder:
            rudderPos = inValue;
            break;
            
          case reg_setThrottle:
            throttlePos = inValue;
            break;
            
          case reg_setDropDoor: // Drop Door Register //
            if(inValue == unlockDoorSignal){ // Unlock door
              doorPos = unlockedPos;
              TelemetryManager->addData(reg_reportedDropDoorState,inValue);
            }
            if(inValue == lockDoorSignal){ // Lock door
              doorPos = lockedPos;
              TelemetryManager->addData(reg_reportedDropDoorState,inValue);
            }
            break;
            
          //case 9:  // Autopilot Toggles //
            /*if(prevAutoMsg == inValue){ // If this message and the last to this register are identical
              controlState = (int) (inValue >> 6); // Set controlState to first 2 bits converted to int
            }else if(prevAutoMsg != 9001){ // Two messages received, both different
              TelemetryManager.addData(groundregCONFLICT,B01000000);
            }
            prevAutoMsg = inValue;*/
            //break; 
        }
      }
      return true; // Success, or message was of 0 length
    }

    void incrementCorruptedMessages(){ corruptedMessages++; };

    int getAileronPos  (){ return aileronPos; }
    int getElevatorPos (){ return elevatorPos; }
    int getRudderPos   (){ return rudderPos; }
    int getThrottlePos (){ return throttlePos; }
    int getDoorPos     (){ return doorPos; }
    
    int getControlState(){ return controlState; }
    int getLoopsPerSecond(){ return (1E3 / avgLoopTime); }
    
  private:
    
    unsigned long avgLoopTime = 0; // Average number of milliseconds each loop takes
    unsigned long flightTime = 0; // Millis since program start
    unsigned long nextTelemetryTime = 0; // Time at which the next telemetry packet should be sent
    
    int corruptedMessages = 0; // Number of invalid messages received since last telemetry broadcast
    int controlState = 0; // 0 = Manual, 1 = Emergency Safety (on radio loss), 2 = Waypoints??, 3 = Full Autopilot??
    
    //RPY planeAngles; // Angles of the plane  

    int aileronPos;
    int elevatorPos;
    int rudderPos;
    int throttlePos;
    int doorPos = unlockedPos; // Door starts unlocked
};

