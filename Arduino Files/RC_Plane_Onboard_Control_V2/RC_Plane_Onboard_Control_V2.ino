/*#include <MPU6050_9Axis_MotionApps41.h>
#include <helper_3dmath.h>
#include <MPU6050.h>
#include <MPU6050_6Axis_MotionApps20.h>*/

/*#include <SX1278.h> oldLora
#include <REMOTEC.h>
#include <AES.h>
#include <LORA.h>*/
// newLora
#include <SPI.h>
//#include <LoRa.h>

#include <Servo.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

//#include <AutoPID.h>

////////// NETWORK //////////

//LORA LR; oldLora
const int network = 1234;
const int cryptoKey = 7703;

const byte big_buffer_len = 69;
byte big_buffer[big_buffer_len];

const int groundDev = 1;
const int planeDev = 2;

////////// STRUCTS //////////

struct RPY {
  int roll;
  int pitch;
  int yaw; 
};

struct sixDofAngles {
  int a1;
  int a2;
  int a3;
  int g1;
  int g2;
  int g3;
};

////////// GROUND REGISTER NUMBERS //////////

int groundregPING = 1;
int groundregROLL = 2;
int groundregPITCH = 3;
int groundregYAW = 4;
int groundregDROP = 5; // Returns drop door state
int groundregLOOP = 6; // Loop time register (avg loop)
int groundregAUTOMODE = 7; // Autopilot state
int groundregCONFLICT = 8; // Conflicting incoming signal warning. #bit1 Drop door, #bit2 control state toggle, #bit3 loops per second out of range. #bit4 out of range int submitted. #bit5 received message of wrong length
int groundregTESTING = 9; // For testing only

////////// TELEMETRY VARIABLES //////////

const int telemetryBufferLength = 64;
const int maxRadioMsg = 32;
byte telemetryBuffer[telemetryBufferLength]; // Defines the buffer for outgoing telemetry
byte telemetryBufferPos = 0;


////////// AEROPLANE CONTROL VARIABLES //////////

//RPY planeAngles; // Angle of the plane

int aileronPos = 90;
int elevatorPos = 10;
int rudderPos = 90;
int throttlePos = 0;

const int lockedPos = 90;
const int unlockedPos = 5;

int doorPos = unlockedPos; // Door starts unlocked

Servo aileron;
Servo elevator;
Servo rudder;
Servo motor;

Servo door;

int controlState = 0; // 0 = Manual, 1 = Emergency Safety (on radio loss), 2 = Waypoints, 3 = Full Autopilot
int batteryCharge;

unsigned long lastSignal; // Time of last signal
unsigned long flightTime = 0; // Micros since program start
long avgLoopTime = 0;

#define controlStateLED 19 // Blue LED under aircraft to signal control state (as of now - nothing for manual, solid for emergency)
#define oneWireBus 9
const long flashSpeed = 500000; // 0.5s
const long telemetryInterval = 500000; // 0.5s - how often non-requested telemetry is sent to ground

////////// SENSOR SETUP //////////

/*OneWire oneWire(oneWireBus); //https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
DallasTemperature sensors(&oneWire);*/

///////////////////// SETUP /////////////////////

void setup(){
  Serial.begin(38400);

  pinMode(controlStateLED,OUTPUT); // Pin 8 - blue wire
  
  // Attach Servos
  motor.attach(2,1000,2000); // ESC wire
  aileron.attach(3); // Brown wire
  door.attach(4); // Red wire
  rudder.attach(5); // Orange wire
  elevator.attach(6); // Red wire
  
  throttlePos = 0;
  elevatorPos = 90;
  aileronPos = 90;
  rudderPos = 90;
  
  // LORA Setup

  /*if(!LR.begin(cryptoKey)){ oldLora
    // Nothing happens
  }*/

  // newLora

  /*if (!LoRa.begin(433E6)) { // 433MHz
    digitalWrite(controlStateLED,HIGH);
    while (true); // Stops the program
  }*/

  /*sensors.begin();*/ 
  
  /*LR.defDevRange(6); oldLora
  LR.defNetAddress(network);*/
}

///////////////////// CORE LOOP /////////////////////

int loops = 0; // Keeps track of how many loops have passed

void loop(){
  
  unsigned long prevFlightTime = flightTime;
  
  // Receive Incoming Data

  /*int msgLength = LR.receiveNetMess(planeDev,groundDev, big_buffer, big_buffer_len); // Receives message, as DEV2 (plane), from DEV1 (ground) oldLora
  if(msgLength > 0){
    lastSignal = micros();
    byte* inMessage = LR.getMessage();
    redirectIncomingData(inMessage, msgLength);
  }*/

  // newLora
  /*int packetSize = LoRa.parsePacket();
  if(packetSize > 0 && packetSize < maxRadioMsg){
    // received a packet
    Serial.print("Received packet of length ");
    Serial.print(packetSize);
    Serial.print(" :");

    byte inMessage[packetSize];
    
    for(int i=0;i<packetSize;i++){
      byte inByte = LoRa.read();
      inMessage[i] = inByte;  
      Serial.print((char) inByte);
    }
    redirectIncomingData(inMessage, packetSize);
    lastSignal = micros();
  }*/

  // Serial alternative
  
  char eol = 0;
  int charOffset = 1;
  
  if(Serial.available()){
    byte inData[64];
    int dataLength = Serial.readBytesUntil(eol,(char*) inData,64);
    
    for(int i=0;i<dataLength;i++){
      int num = ((int) inData[i]) - charOffset;
      inData[i] = (byte) num;
    }
    redirectIncomingData(inData, dataLength);
    lastSignal = micros();
  }

  // Handles emergency mode
  
  flightTime = micros();
  
  controlState = getControlState(); // Gets the control state
  if(controlState == 1){ // 1 = emergency mode
    emergencyControlManager();
    digitalWrite(controlStateLED,HIGH);
  }else{
    digitalWrite(controlStateLED,LOW);
  }

  // Updates controls
  aileron.write(aileronPos);
  elevator.write(elevatorPos);
  rudder.write(rudderPos);
  door.write(doorPos);
  motor.write(throttlePos);

  // Handles Telemetry
  
  unsigned long timeDif = flightTime-prevFlightTime;
  avgLoopTime = (avgLoopTime*0.9) + (timeDif*0.1); // Causes the new time to only mildly affect results
  
  if((flightTime % telemetryInterval) < (prevFlightTime % telemetryInterval)){ // If it is time to send telemetry
    /* Loop time calculations
    if(avgLoopTime < 1000000){ // Less than 1000ms
      double x = (avgLoopTime*1.0)/1000000.0; // provides a 0-1 number
      int num = 254-(pow(254,(1-x))); // Creates a curve of numbers so there are more output values for smaller loop times
      addTelemetryInt(groundregLOOP,num);
    }else{ // More than 1 second
      addTelemetryInt(groundregLOOP,254);
    }*/
    int loopSpeed = round((1000000/telemetryInterval) * loops / 10); // Divides 1 second by how often this is calculated to get loops per second
    if(loopSpeed > 254){
      addTelemetryInt(groundregLOOP,254);
    }else{
      addTelemetryInt(groundregLOOP,loopSpeed);
    }
    
    loops = 0;

    // Emergency status:
    if(controlState == 1){ // emergency mode
      addTelemetry(groundregAUTOMODE,B11110000);
    }else if(controlState == 0){ // normal mode
      addTelemetry(groundregAUTOMODE,B00001111);
    }

    // Sensors:
    /*sensors.requestTemperatures();
    sensors.getTempCByIndex(0);*/
    // Send all telemetry data
    sendTelemetry();
  }
  
  loops++;
}

///////////////////// DATA CONTROL /////////////////////

void redirectIncomingData(byte inBytes[], int inLength){
  if((inLength%2)!=0){ // If array is not a multiple of 2, an error has occurred
    addTelemetry(groundregCONFLICT,B00001000);
    return;
  }

  int messageLength = inLength/2; // Number of commands from ground
  
  // Define the two arrays to write to
  int inRegisters[messageLength]; // Number of registers is the same as number of values
  int inValues[messageLength];

  for(int i=0;i<inLength;i+=2){
    inRegisters[i/2] = (int) inBytes[i];
    inValues[i/2] = (int) inBytes[i+1];
  }

  // Storing variables for messages that need multiple sends to go through (safety mechanism)
  int prevDropMsg = 9001;
  int prevAutoMsg = 9001;
  
  for(int i=0;i<messageLength;i++){ // Cycles through all incoming registers
     switch(inRegisters[i]){
      case 0: // Nothing sent to this register - do nothing
        break;
      case 1:  // Aileron Register //
        aileronPos = inValues[i];
        //Serial.println("changing ailerons");
        break;
      case 2:  // Elevator Register //
        elevatorPos = inValues[i];
        //Serial.println("changing Elevator");
        break;
      case 3:  // Rudder Register //
        rudderPos = inValues[i];
        //Serial.println("changing Rudder");
        break;
      case 4:  // Throttle Register //
        throttlePos = inValues[i];
        break;
      case 5: // Drop Door Register //
        if(prevDropMsg == inValues[i]){ // If this message and the last to this register are identical
          //Serial.println("changing Drop Door");
          if(inValues[i] == 100){ // Unlock door
            doorPos = unlockedPos;
            addTelemetry(groundregDROP,inValues[i]);
          }
          if(inValues[i] == 200){ // Lock door
            doorPos = lockedPos;
            addTelemetry(groundregDROP,inValues[i]);
          }
        }else if(prevDropMsg != 9001){ // Two messages received, both different
          addTelemetry(groundregCONFLICT,B10000000);
        }
        prevDropMsg = inValues[i];
        break;
      case 6: // LED Register
        break;
      case 7: // Other Onboard Controls //
        break;
      case 8: // Other Onboard Controls //
        break;
      case 9:  // Autopilot Toggles //
        /*if(prevAutoMsg == inValues[i]){ // If this message and the last to this register are identical
          controlState = (int) (inValues[i] >> 6); // Set controlState to first 2 bits converted to int
        }else if(prevAutoMsg != 9001){ // Two messages received, both different
          addTelemetry(groundregCONFLICT,B01000000);
        }
        prevAutoMsg = inValues[i];*/
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
}

///////////////////// SAFETY AND AUTOPILOT CONTROLS /////////////////////

void emergencyControlManager(){ // Manages the flight surfaces in emergency and runs PID
  // This function will run the PID loop
  
  throttlePos = 0;   // Shuts off the motor
  elevatorPos = 100; // Tilts the plane slightly up to lose speed and glide into a landing
  aileronPos = 100; // Banks slightly
  rudderPos = 90; // Stays central
  
}

/*RPY returnTargetAngles(){ // Will provide targets for the PID loop in emergencyControlManager
  // This function is where any autopilot "planning" will happen in the future
  
  // Predefined values
  int roll = 0;
  int pitch = 5;
  int yaw = 0;

  RPY testValues = {roll,pitch,yaw};
  return testValues;
}*/

int getControlState(){
  // May have more complex criterea in future - just now it just waits 3 secs
  int state = controlState;
  if(flightTime-lastSignal > 3000000){ // If last signal was more than 3 seconds ago
    state = 1; // Emergency
  }else{
    state = 0; // Manual
  }
  return state;
}

///////////////////// TELEMETRY AND MISCELLANEOUS /////////////////////

void sendTelemetry(){
  // Sends Message
  
  //LR.sendNetMess(groundDev,planeDev,telemetryBuffer,telemetryBufferPos); // Sends message to DEV1 (ground) from DEV2 (plane)
  //LR.receiveMessMode(); // Returns to receive mode
  

  // Serial alternative
  
  char eol = 0;
  int charOffset = 1;
  
  String message = "";
  
  for(int i=0;i<telemetryBufferPos;i++){
    int num = (int) telemetryBuffer[i]; // The number sent
    num += charOffset; // Adds charOffset for sending
    message += (char) num; // Adds the new number to the message
  }
  
  int val = telemetryBufferPos+charOffset;
  
  /*message += (char) (groundregTESTING+charOffset);
  message += (char) val;*/
  
  message += eol;
  Serial.print(message); // Sends the message to PI
  
  telemetryBufferPos = 0;
}

void addTelemetry(byte reg,byte val){
  telemetryBuffer[telemetryBufferPos] = reg;
  telemetryBuffer[telemetryBufferPos+1] = val;
  telemetryBufferPos += 2;
}

void addTelemetryInt(int reg,int val){
  
  if((val > 254) || (val < 0)){ // Outside permissable range
    if(reg == groundregLOOP){
      addTelemetry(groundregCONFLICT,B00100000); // Loops per second too high to send.
    }else{
      addTelemetry(groundregCONFLICT,B00010000); // Generic byte too high
    }
    return;
  }
  telemetryBuffer[telemetryBufferPos] = (byte) reg;
  telemetryBuffer[telemetryBufferPos+1] = (byte) val;
  telemetryBufferPos += 2;
}

////////// Bank Angles //////////
int calculateRPY(){
  // returns: Roll, Pitch, Yaw
  
}

int returnMPU6050(){
  // returns: accellerometer, gyros
  
}

