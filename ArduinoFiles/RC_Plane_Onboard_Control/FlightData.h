
#define lockedPos 70;
#define unlockedPos 150;

class FLIGHT_DATA {
  public:
    RADIO* Radio; // The plane's onboard radio class
    SENSOR_MANAGER* SensorManager; // The plane's sensor manager
    DATA_MANAGER* TelemetryManager; // The class which manages the telemetry buffer
    
    FLIGHT_DATA (RADIO* planeRadio, SENSOR_MANAGER* planeSensors, DATA_MANAGER* planeTelemetry) : Radio(planeRadio), SensorManager(planeSensors), TelemetryManager(planeTelemetry) {
    }

    bool beginRadio(){
      return Radio->begin();
    }
    
    void updateFlightData(){
      loopsSinceTelemetry++;
      flightTime = millis();
      /*unsigned long timeDif = millis() - flightTime;
      avgLoopTime = (avgLoopTime + timeDif) / 2; // Take rolling average of loop times
      Serial.print("AvgLoopTime: "); Serial.println(avgLoopTime);*/
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
      
      if(millis()-lastSignal > 2000){ // If last signal was more than 2 seconds ago
        controlState = 1; // Signal Loss: Emergency
      }else{
        controlState = 0; // Manual
      }
      return controlState;
    }

    void updateSensorReadings(){
      SensorManager->updateReadings();
    }

    bool sendTelemetry(){
      /*if (flightTime < (lastTelemetryTime+telemetryInterval)){ // (lastTelemetryTime+telemetryInterval) is the timestamp at which the next telemetry packet should be sent
        return false; // No telemetry sent
      }*/
      
      float timeSinceTelemetry = flightTime - lastTelemetryTime;
      float timePerLoop = timeSinceTelemetry / loopsSinceTelemetry;
      int loopsPerSecond = constrain( timePerLoop, 0, 255 );
      
      // Reset counters:
      loopsSinceTelemetry = 0;
      lastTelemetryTime = millis();
      
      TelemetryManager->addData(reg_onboardLoopSpeed,loopsPerSecond);
      
      TelemetryManager->addData(reg_onboardRSSI, Radio->getAvgRSSI()); // Received signal strength
  
      TelemetryManager->addData(reg_reportedControlState, getControlState()); // Control Status (normal or "autopilot")
  
      // Add telemetry for number of corrupted messages
      // Sensors:
      TelemetryManager->addData(reg_currentBattVoltage, SensorManager->getBatteryVoltage());
      /*sensors.requestTemperatures();
      sensors.getTempCByIndex(0);*/
      
      // Send all telemetry data
      char* outDataBuffer; // Will point to the beginning of the telemetryBuffer array
      int outDataLength; // Will contain length of the data in the telemetryBuffer array
      TelemetryManager->getData(&outDataBuffer,&outDataLength);
      Radio->transmitData(outDataBuffer, outDataLength); // After this line, *outDataBuffer and outDataLength go out of scope.

      return true; // Telemetry sent
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

          case reg_requestTelemetry:
            // Ground requests telemetry
            
            if(inValue == reg_requestTelemetry){
              sendTelemetry();
              Serial.println("HELLO!");
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
    int getLoopsPerSecond(){ return loopsSinceTelemetry / ((flightTime - lastTelemetryTime)/1000); }
    
  private:
    
    int loopsSinceTelemetry = 0;
    unsigned long flightTime = 0; // Millis since program start
    unsigned long lastTelemetryTime = 0; // Millis Time at which the previous telemetry packet was sent
    
    int corruptedMessages = 0; // Number of invalid messages received since last telemetry broadcast
    int controlState = 0; // 0 = Manual, 1 = Emergency Safety (on radio loss), 2 = Waypoints??, 3 = Full Autopilot??
    
    //RPY planeAngles; // Angles of the plane  

    int aileronPos;
    int elevatorPos;
    int rudderPos;
    int throttlePos;
    int doorPos = lockedPos; // Door starts locked
};

