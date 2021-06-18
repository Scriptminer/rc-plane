
#define lockedPos 90;
#define unlockedPos 5;

class FLIGHT_DATA {
  public:
    RADIO* Radio; // The plane's onboard radio class
    SENSOR_MANAGER* SensorManager; // The plane's sensor manager
    TELEMETRY_MANAGER* TelemetryManager; // The class which manages the telemetry buffer
    
    FLIGHT_DATA (RADIO* planeRadio, SENSOR_MANAGER* planeSensors, TELEMETRY_MANAGER* planeTelemetry) : Radio(planeRadio), SensorManager(planeSensors), TelemetryManager(planeTelemetry) {
    }

    bool beginRadio(unsigned long frequency, int txPower){
      return Radio->begin(frequency,txPower);
    }
    
    void updateFlightData(){
      prevFlightTime = flightTime;
      flightTime = micros();
      unsigned long timeDif = flightTime-prevFlightTime;
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
      if(flightTime-Radio->getLastSignal() > 2E6){ // If last signal was more than 2 seconds ago
        controlState = 1; // Signal Loss: Emergency
      }else{
        controlState = 0; // Manual
      }
      return controlState;
    }

    bool timeForTelemetry(unsigned long telemetryInterval){
      if( (flightTime % telemetryInterval) < (prevFlightTime % telemetryInterval) ){ return true; }
      else{ return false; }
    }

    bool handleIncomingData(byte inBytes[], int inLength){
      if((inLength%2)!=0){ // If array is not a multiple of 2, an error has occurred
        return false; // Failure
      }
      
      // Storing variables for messages that need multiple sends to go through (safety mechanism)
      int prevDropMsg = 9001;
      int prevAutoMsg = 9001;

      int messageLength = inLength/2; // Number of commands from ground
      
      for(int i=0;i<messageLength;i++){ // Cycles through all incoming registers
         int inRegister = (int) inBytes[i*2];
         int inValue = (int) inBytes[(i*2)+1];
         switch(inRegister){
          case 0: // Nothing sent to this register - do nothing
            break;
          case 1:  // Aileron Register //
            aileronPos = inValue;
            //Serial.println("changing ailerons");
            break;
          case 2:  // Elevator Register //
            elevatorPos = inValue;
            //Serial.println("changing Elevator");
            break;
          case 3:  // Rudder Register //
            rudderPos = inValue;
            //Serial.println("changing Rudder");
            break;
          case 4:  // Throttle Register //
            throttlePos = inValue;
            break;
          case 5: // Drop Door Register //
            if(prevDropMsg == inValue){ // If this message and the last to this register are identical
              //Serial.println("changing Drop Door");
              if(inValue == 100){ // Unlock door
                doorPos = unlockedPos;
                TelemetryManager->addTelemetry(groundregDROP,inValue);
              }
              if(inValue == 200){ // Lock door
                doorPos = lockedPos;
                TelemetryManager->addTelemetry(groundregDROP,inValue);
              }
            }else if(prevDropMsg != 9001){ // Two messages received, both different
              TelemetryManager->addTelemetry(groundregCONFLICT,B10000000);
            }
            prevDropMsg = inValue;
            break;
          case 6: // LED Register
            break;
          case 7: // Other Onboard Controls //
            break;
          case 8: // Other Onboard Controls //
            break;
          case 9:  // Autopilot Toggles //
            /*if(prevAutoMsg == inValue){ // If this message and the last to this register are identical
              controlState = (int) (inValue >> 6); // Set controlState to first 2 bits converted to int
            }else if(prevAutoMsg != 9001){ // Two messages received, both different
              TelemetryManager.addTelemetry(groundregCONFLICT,B01000000);
            }
            prevAutoMsg = inValue;*/
            break;
          case 10:
            break;
          case 11: // Setup Register
            break;
          case 12:
            break;
          case 13:
            break;
          case 14:
            break;
          case 15:
            break;       
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
    int getLoopsPerSecond(){ return (1E6 / avgLoopTime); }
    
  private:
    
    unsigned long avgLoopTime = 0;
    unsigned long flightTime = 0; // Micros since program start
    unsigned long prevFlightTime = 0; // Uptime of this program
    unsigned long lastTelemetryBroadcast = 0;
    
    int corruptedMessages = 0; // Number of invalid messages received since last telemetry broadcast
    int controlState = 0; // 0 = Manual, 1 = Emergency Safety (on radio loss), 2 = Waypoints??, 3 = Full Autopilot??
    
    //RPY planeAngles; // Angles of the plane  

    int aileronPos;
    int elevatorPos;
    int rudderPos;
    int throttlePos;
    int doorPos = unlockedPos; // Door starts unlocked
};

