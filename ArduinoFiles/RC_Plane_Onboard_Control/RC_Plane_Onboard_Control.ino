
/*#include <MPU6050_9Axis_MotionApps41.h>
#include <helper_3dmath.h>
#include <MPU6050.h>
#include <MPU6050_6Axis_MotionApps20.h>*/

// newLora
#include <SPI.h>
#include <LoRa.h>

#include <Servo.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>

//#include <AutoPID.h>

#include "Radio.h"
#include "CommonConstants.h"
#include "Sensors.h"
#include "DataManager.h"
#include "FlightData.h"

Servo aileron;
Servo elevator;
Servo rudder;
Servo motor;

Servo door;

/*RADIO __tmpRadio();
SENSOR_MANAGER __tmpSensorManager();
DATA_MANAGER __tmpTelemetryManager(new byte[64], 64);
*/

FLIGHT_DATA ThisFlight(new RADIO(maxRadioMessageLength),
                       new SENSOR_MANAGER(),
                       new DATA_MANAGER(new byte[maxRadioMessageLength], maxRadioMessageLength)
                      );

#define controlStateLED 19 // Blue LED under aircraft to signal control state (as of now - nothing for manual, solid for emergency)
#define oneWireBus 9

const long flashSpeed = 500; // 0.5s
const long telemetryInterval = 500; // 0.5s - how often non-requested telemetry is sent to ground

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
  
  // LoRa Setup:
  if(!ThisFlight.beginRadio(433E6,2)){ // Frequency, txPower in dBm (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW)
    digitalWrite(controlStateLED,HIGH);
    while (true); // Stops the program
  }

  /*sensors.begin();*/ 
}

///////////////////// CORE LOOP /////////////////////

int loops = 0; // Keeps track of how many loops have passed

void loop(){
  // Receive Incoming Data
  static byte inDataBuffer[maxRadioMessageLength];
  int inDataLength;
  ThisFlight.Radio->receiveData(inDataBuffer,&inDataLength);
  if(!ThisFlight.handleIncomingData(inDataBuffer, inDataLength)){
    // Error parsing data:
    ThisFlight.incrementCorruptedMessages();
  }

  
  // Handles emergency mode
  ThisFlight.updateFlightData();
  
  if(ThisFlight.updateControlState( ThisFlight.Radio->getLastSignal() ) == 1){ // 1 = emergency mode
    ThisFlight.updateControls(ThisFlight.getControlState()); // Will change servo settings if necessary in emergency
    digitalWrite(controlStateLED,HIGH);
  }else{
    digitalWrite(controlStateLED,LOW);
  }

  // Updates controls
  aileron.write(ThisFlight.getAileronPos());
  elevator.write(ThisFlight.getElevatorPos());
  rudder.write(ThisFlight.getRudderPos());
  door.write(ThisFlight.getDoorPos());
  motor.write(ThisFlight.getThrottlePos());

  // Handles Telemetry
  if(ThisFlight.timeForTelemetry(telemetryInterval)){ // If it is time to send telemetry
    int loopsPerSecond = constrain( ThisFlight.getLoopsPerSecond(), 0, 255 );
    ThisFlight.TelemetryManager->addData(reg_onboardLoopSpeed,loopsPerSecond);
    
    ThisFlight.TelemetryManager->addData(reg_onboardRSSI,ThisFlight.Radio->getAvgRSSI()); // Received signal strength

    ThisFlight.TelemetryManager->addData(reg_reportedControlState,ThisFlight.getControlState()); // Control Status (normal or "autopilot")

    // Add telemetry for number of corrupted messages
    // Sensors:
    ThisFlight.TelemetryManager->addData(reg_currentBattVoltage,ThisFlight.SensorManager->getBatteryVoltage());
    /*sensors.requestTemperatures();
    sensors.getTempCByIndex(0);*/
    
    // Send all telemetry data
    char* outDataBuffer; // Will point to the beginning of the telemetryBuffer array
    int outDataLength; // Will contain length of the data in the telemetryBuffer array
    ThisFlight.TelemetryManager->getData(&outDataBuffer,&outDataLength);
    ThisFlight.Radio->transmitData(outDataBuffer, outDataLength); // After this line, *outDataBuffer and outDataLength go out of scope.
    
  }
}
