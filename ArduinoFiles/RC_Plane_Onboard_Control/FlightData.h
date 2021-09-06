
#define lockedPos 70; // Servo positions for the drop door
#define unlockedPos 150;

class FlightData {
  public:
    Radio* radio; // The plane's onboard radio class
    SensorManager* sensorManager; // The plane's sensor manager
    DataManager* telemetryManager; // The class which manages the telemetry buffer
    
    FlightData (Radio* planeRadio, SensorManager* planeSensors, DataManager* planeTelemetry) : radio(planeRadio), sensorManager(planeSensors), telemetryManager(planeTelemetry) {
    }

    bool beginRadio(){
      return radio->begin();
    }
    
    void updateFlightData(){
      // Update any flight status information variables that needs updated every loop.
      loopsSinceTelemetry++;
    }

    void updateControls(){
      // Modifies the values to be written to the servos based on the current control state.
      switch(controlState){
        case 0: // Manual flight:
          break; // No modifications needed to existing control positions
        
        case 1: // Emergency static controls:
          // Sets all the controls to fixed "safe" positions. Most importantly, shutting off the motor.
          throttlePos = 0;   // Shuts off the motor
          elevatorPos = 100; // Tilts the plane slightly up to lose speed and glide into a landing
          aileronPos = 100; // Banks slightly
          rudderPos = 90; // Stays central
          digitalWrite(CONTROL_STATE_LED,HIGH);
          break;
        case 2: // Emergency hold attitude
          break; // In future, may call a PID loop in the Autopilot class to hold the plane's attitude level based on sensor readings.
        default:
          // Should not occur.
          digitalWrite(CONTROL_STATE_LED,HIGH);
          break;
      }
    }

    int updateControlState(unsigned long lastSignal){
      // Determine which control state the plane should currently be operating in, based on the time since the last inbound radio packet.
      if(millis()-lastSignal > 3000){ // If last signal was more than 3 seconds ago. 3 seconds was chosen as a compromise between control takeover being too quick and being a risking causing a crash due to interfering with a manoever, and too slow, risking the plane crashing into something with the engine likely on.
        controlState = 1; // Signal Loss: Emergency
      }else{
        controlState = 0; // Manual
      }
      return controlState;
    }

    void updateSensorReadings(){
      // Updates the average readings of various sensors.
      sensorManager->updateReadings();
    }
    
    bool sendTelemetry(){
      // Transmit telemetry to ground and empty telemetry buffer.
      
      float timeSinceTelemetry = millis() - lastTelemetryTime;
      float secondsSinceTelemetry = timeSinceTelemetry / 1000.0;
      
      telemetryManager->addData(reg_onboardLoopSpeed,getAvgLoopTime());
      Serial.println(getAvgLoopTime());
      
      telemetryManager->addData(reg_onboardRSSI, abs(radio->getAvgRSSI())); // Received signal strength

      float radioPacketsPerSecond = ((float) radioPacketsSinceLastTelemetry) / secondsSinceTelemetry;
      telemetryManager->addData(reg_onboardPacketReceiveRate, (int) radioPacketsPerSecond);
      radioPacketsSinceLastTelemetry = 0; // Reset radio packets counter.
      
      telemetryManager->addData(reg_reportedControlState, getControlState()); // Control Status (normal or "autopilot")
  
      // Add telemetry for number of corrupted messages
      // Sensors:
      telemetryManager->addData(reg_currentBattVoltage, sensorManager->getBatteryVoltage());
      /*sensors.requestTemperatures();
      sensors.getTempCByIndex(0);*/
      
      // Send all telemetry data
      char* outDataBuffer; // Will point to the beginning of the telemetryBuffer array
      int outDataLength; // Will contain length of the data in the telemetryBuffer array
      telemetryManager->getData(&outDataBuffer,&outDataLength);
      telemetryManager->resetDataBuffer(); // Resets the internal buffer pointer, ready for new data.
      radio->transmitData(outDataBuffer, outDataLength); // After this line, *outDataBuffer and outDataLength go out of scope.

      // Reset counters:
      loopsSinceTelemetry = 0;
      lastTelemetryTime = micros() / 1000.0;
      
      return true; // Telemetry sent
    }

    bool handleIncomingData(byte inBytes[], int inLength){
      // Reads the incoming register-value pairs and takes appropriate action for each.
      if((inLength%2)!=0){ // If array is not a multiple of 2, an error has occurred
        return false; // Failure
      }

      if(inLength > 0){
        radioPacketsSinceLastTelemetry++; // Increment the number of packets received since the previous telemetry request.
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
              telemetryManager->addData(reg_reportedDropDoorState,inValue);
            }
            if(inValue == lockDoorSignal){ // Lock door
              doorPos = lockedPos;
              telemetryManager->addData(reg_reportedDropDoorState,inValue);
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
              telemetryManager.addData(groundregCONFLICT,B01000000);
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
    
  private:
    
    int loopsSinceTelemetry = 0;
    float lastTelemetryTime = 0; // Millis Time at which the previous telemetry packet was sent
    
    int corruptedMessages = 0; // Number of invalid messages received since last telemetry broadcast
    int radioPacketsSinceLastTelemetry = 0;
    int controlState = 0; // 0 = Manual, 1 = Emergency Safety (on radio loss), 2 = Waypoints??, 3 = Full Autopilot??
    
    //RPY planeAngles; // Angles of the plane  

    int aileronPos;
    int elevatorPos;
    int rudderPos;
    int throttlePos;
    int doorPos = lockedPos; // Door starts locked

    int getAvgLoopTime(){
      float timeSinceTelemetry = (micros()/1000.0) - lastTelemetryTime; // Milliseconds since telemetry
      timeSinceTelemetry = timeSinceTelemetry * 10; // Hectomicroseconds (0.1ms units) since telemetry
      return (int) (timeSinceTelemetry / loopsSinceTelemetry);
    }
};

