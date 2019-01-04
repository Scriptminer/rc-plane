
#include <LORA.h>

/*** Network values ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383
#define KEYVAL 111       // criptography key (must be the same for all devices.

/*** sender address ***/
#define MYADD 4  

/*** receiver address ***/
#define TOADD 6


////////// TELEMETRY VARIABLES //////////

const int telemetryBufferLength = 64;
byte telemetryBuffer[telemetryBufferLength]; // Defines the buffer for outgoing telemetry
byte telemetryBufferPos = 0;

void addTelemetry(byte reg,byte val){
  telemetryBuffer[telemetryBufferPos] = reg;
  telemetryBuffer[telemetryBufferPos+1] = val;
  telemetryBufferPos += 2;
}


LORA LR;             //class instance

void setup() 
{
  Serial.begin(115200);
  pinMode(A5,OUTPUT);
  
  long startSetupTime = millis();
  setupLoRa();
  
  if (!LR.begin(KEYVAL)) //initialise LoRa radio
    {Serial.println("LoRa.begin() Failed! Stopping!");return;}

  configLoRaUplink(); // Current setup broadcasts uplink signals only.
  Serial.print("Setup successful, took: "); Serial.print(millis()-startSetupTime); Serial.println("ms.");
  Serial.println("LoRa 16bit counter transmitter ready...");
}

void setupLoRa(){
  // Does all the setup prior to beginning running LoRa
  LR.setConfig(7,3,4); // Sets the spreading factor to 7, bandwidth BW = 3 -> 20.8kHz (legal max is 25kHz)

  /* Network definition */
  LR.defDevRange(DEVRANGE);
  LR.defNetAddress(LORANET); 
  LR.receiveMessMode();   // radio moldule in receiver mode 
}

void configLoRaUplink(){
  // Configures LoRa for the ground to plane link.
  LR.setFrequency(459); // 459MHz, should be within the 458.5-459.5MHz telecommand band - legal for uplink ONLY.
  LR.setPower(1); // PW = 5 -> 20dBm -> 100mW, maximum legal power for telecommand
}

void configLoRaDownlink(){
  // Configures LoRa for the plane to ground link.
  LR.setFrequency(434.04); // 434.04MHz, first channel within the 434.04 - 434.79 MHz telemetry band.
  LR.setPower(1); // PW = 2 -> 10dBm -> 10mW, maximum legal power for telemetry 
}

unsigned int counter = 0;

void loop(){
  byte high = (counter & 0xFF00) >> 8;
  byte low = counter & 0x00FF;
  
  addTelemetry(1,high);
  addTelemetry(2,low);
  addTelemetry(3,high);
  addTelemetry(4,low);
  counter++;
  
  digitalWrite(A0,HIGH); // Turn LED on to show how long sending takes.
  LR.sendNetMess(TOADD,MYADD,telemetryBuffer,telemetryBufferPos);
  digitalWrite(A0,LOW); // Turn LED off
  telemetryBufferPos = 0; // Restarts buffer for new data
}
