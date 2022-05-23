
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



////////// SENSOR SETUP //////////

/*OneWire oneWire(oneWireBus); //https://create.arduino.cc/projecthub/TheGadgetBoy/ds18b20-digital-temperature-sensor-and-arduino-9cc806
DallasTemperature sensors(&oneWire);*/

///////////////////// SETUP /////////////////////

// Pins the signal wire bus which runs the length of the plane is attached to:
// Violet, Grey and White wires on the bus are currently not connected to the NANO.
#define BROWN_WIRE 3
#define RED_WIRE 4
#define ORANGE_WIRE 5
#define YELLLOW_WIRE 6
#define GREEN_WIRE 7
#define BLUE_WIRE 8

// Other NANO pins:
#define CONTROL_STATE_LED 8
#define ESC_WIRE 2

// Define the servos:
Servo aileron;
Servo elevator;
Servo rudder;
Servo motor;
Servo door;

#include "CommonConstants.h"
#include "Radio.h"
#include "Sensors.h"
#include "DataManager.h"
#include "FlightData.h"

// Initialise the Flight Manager Class:
FlightData flightManager(new Radio(maxRadioMessageLength,groundToAirFrequency,airToGroundFrequency,groundToAirBandwidth,airToGroundBandwidth,airToGroundTxPower), // maxMessageLength, rxFrequency, txFrequency, rxBandwidth, txBandwidth, txPower in dBm 
                         new SensorManager(),
                         new DataManager(new byte[maxRadioMessageLength], maxRadioMessageLength)
                      );

void setup(){
  Serial.begin(38400);
  pinMode(CONTROL_STATE_LED,OUTPUT);
  
  // Attach Servos
  motor.attach(ESC_WIRE,1000,2000); // ESC wire. ESC uses nonstandard servo pulse width, so needs these values set manually.
  aileron.attach(BROWN_WIRE);
  door.attach(RED_WIRE);
  rudder.attach(ORANGE_WIRE);
  elevator.attach(YELLLOW_WIRE);
  
  // LoRa Setup:
  if(!flightManager.beginRadio()){
    digitalWrite(CONTROL_STATE_LED,HIGH);
    Serial.println("LoRa setup failed.");
    while (true); // Stops the program
  }
  Serial.println("LoRa started.");

  /*sensors.begin();*/ 
}

///////////////////// CORE LOOP /////////////////////

void loop(){
  // Receive Incoming Data
  static byte inDataBuffer[maxRadioMessageLength];
  int inDataLength = 0;
  flightManager.radio->receiveData(inDataBuffer,&inDataLength);

  if(!flightManager.handleIncomingData(inDataBuffer, inDataLength)){
    // Error parsing data:
    flightManager.incrementCorruptedMessages();
  }
  
  flightManager.updateFlightData();
  
  // Handles emergency mode
  flightManager.updateControlState(flightManager.radio->getLastSignal()); // Control state is currently determined only by the time since the last radio message. (less than a given threshold manual, over a given threshold emergency)
  flightManager.updateControls(); // Modifies the control positions if the controlState is not in manual flight mode (i.e. emergency mode)
  if( flightManager.getControlState() == 1 ){ // 1 is emergency control takeover mode
    
    digitalWrite(CONTROL_STATE_LED,HIGH);
  }else{
    digitalWrite(CONTROL_STATE_LED,LOW);
  }

  // Updates controls
  aileron.write(flightManager.getAileronPos());
  elevator.write(flightManager.getElevatorPos());
  rudder.write(flightManager.getRudderPos());
  door.write(flightManager.getDoorPos());
  motor.write(flightManager.getThrottlePos());
  
  // Update sensor readings
  flightManager.updateSensorReadings();

}
