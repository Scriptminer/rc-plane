//#include <LoRa.h>

#include <SPI.h>

/*#include <SX1278.h> oldLora
#include <REMOTEC.h>
#include <AES.h>
#include <LORA.h>*/

////////// NETWORK //////////

// LORA LR; oldLora
const int resetPin = 1;
const int network = 1234;
const int cryptoKey = 7703;

const byte big_buffer_len = 69;
byte big_buffer[big_buffer_len];

const int groundDev = 1;
const int planeDev = 2;

byte inDataBuffer[64];
int dataBufferCounter = 0;

// For communication with Pi
char eol = '\n';
int charOffset = 1;

String message = ""; // Next message to be sent

///////////////////// SETUP /////////////////////

void setup() {

  Serial.begin(38400);

  /*if(!LR.begin(cryptoKey)){ oldLora
    Serial.println("Not beginning...");
  }
  LR.defDevRange(6);
  LR.defNetAddress(network);*/
  
  /*if (!LoRa.begin(433E6)) { TEMPORARY COMMENT
    message += (char) 130; // Register for radio startup +1
    message += (char) 201;
  }else{
    message += (char) 130; // Register for radio startup +1
    message += (char) 101;
  }*/
}

unsigned long lastSignal = 0;

unsigned long thisSecond = 0;
int loops = 0;
int averageLoopSpeed;

///////////////////// CORE LOOP /////////////////////

void loop() {
  
  // UNO-PI Telemetry:
  /*if(lastSignal < 2000000){ // Less than 2000ms
    double x = (lastSignal*1.0)/2000000.0; // provides a 0-1 number
    int num = 254-(pow(254,(1-x))); // Creates a curve of numbers so there are more output values for smaller loop times
    message += (char) num;
  }else{ // More than 1 second
    message += (char) 255;
  }*/
  if(micros()%1000 > thisSecond){
    thisSecond = micros();
    message += (char) 129; // Register for loop speed +1
    message += char(loops+1);
  }
  
  
  // Ground-plane Forwarding:
  if(Serial.available()){
    byte inData[64];
    int dataLength = Serial.readBytesUntil(eol,(char*) inData,64);

    Serial.print("Reading Serial of length ");
    Serial.println(dataLength);
    
    for(int i=0;i<dataLength;i++){
      int num = ((int) inData[i]) - charOffset;
      inData[i] = (byte) num;
      Serial.print(num);
      Serial.print(",");
    }
    Serial.println("");
    
    /* Serial alternative:
    Serial.println("Final Output Array Values:");
    for(int i=0;i<dataLength;i++){
      Serial.println(inData[i]);
    }
    Serial.println("------");*/
    /*LoRa.beginPacket(); TEMPORARY COMMENT
    LoRa.write(inData,dataLength);
    LoRa.endPacket();*/
    /*LR.sendNetMess(planeDev,groundDev,inData,dataLength); // Sends message to DEV2 (plane) from DEV1 (ground) oldLora
    LR.receiveMessMode(); // Returns to receive mode */
  }

  // Plane-Ground Forwarding:
  
  /* Serial alternative
  int msgLength = dataLength;
  byte* inMessage = inData;*/
  
  /*int msgLength = LR.receiveNetMess(groundDev, planeDev, big_buffer, big_buffer_len); // Finds length of incoming message
  
  if(msgLength > 0){ // If there is a message oldLora
    byte* inMessage = LR.getMessage(); // Receives message, as DEV1 (ground), from DEV2 (plane)
    
    lastSignal = micros();

    for(int i=0;i<msgLength;i++){
      int num = (int) inMessage[i]; // The number sent
      num += charOffset; // Adds charOffset for sending
      message += (char) num; // Adds the new number to the message
    }
  }*/
  // newLora
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet of length ");
    Serial.print(packetSize);
    Serial.print(" :");

    // read packet
    while(LoRa.available()){
      Serial.print(",");
      int num = (int) LoRa.read(); // The number sent
      num += charOffset; // Adds charOffset for sending
      message += (char) num; // Adds the new number to the message
      Serial.print(".");
    }
    
    message += eol;
    Serial.print(message); // Sends the message to PI 
    Serial.println("");
  }
  
  loops++;
  message = ""; 
}
