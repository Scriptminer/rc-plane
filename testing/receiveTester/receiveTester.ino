
#include <LORA.h>

/*** Network values ***/
#define DEVRANGE 4       // 1 to 15 
#define LORANET 333      // choosed from 0 to 16383
#define KEYVAL 111       // criptography key (must be the same for all devices.

/*** receiver address ***/
#define MYADD 6  

#define reclen 64   //receiver buffer
char recbuff[reclen];

LORA LR;             //class instance

void setup(){
  Serial.begin(115200);
  pinMode(A5,OUTPUT);
  
  long startSetupTime = millis();
  setupLoRa();
  
  if (!LR.begin(KEYVAL)) //initialise LoRa radio
    {Serial.println("LoRa.begin() Failed! Stopping!");return;}

  configLoRaUplink(); // Current setup listens for uplink signals only.
  Serial.print("Setup successful, took: "); Serial.print(millis()-startSetupTime); Serial.println("ms.");
  Serial.println("Beginning 10 second analysis of incoming data...");
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

/*void loop() // Original Loop Code
{
  
  int inMsgLen = LR.receiveNetMess(MYADD,0,(byte*)recbuff,reclen); //receive from anyone on net
  
  if (inMsgLen>0) 
  {
    //int sender=LR.getNetSender();
    digitalWrite(A5,HIGH);
    unsigned char *mess=LR.getMessage();
    digitalWrite(A5,LOW);
    
    String msg = "";
    for(int i=0;i<inMsgLen;i++){
      int num = mess[i];
      msg += String(num);
      msg += ", ";
    }
    Serial.print("From: ");Serial.print(" > ");Serial.println(msg);
  }
}*/

int messagesReceived = 0;
int messagesMissed = 0;

int valueErrors = 0;
int lengthErrors = 0;

unsigned int prevValue = 0;

void loop(){ // Field Testing Variant Code
  static long startTime = millis(); // Only written to at beginning of first loop
  static bool firstMessage = true; // Used to determine whether a message is the first received in this test

  int inMsgLen = LR.receiveNetMess(MYADD,0,(byte*)recbuff,reclen); //receive from anyone on net
  
  if (inMsgLen>0) {
    //int sender=LR.getNetSender();
    digitalWrite(A5,HIGH); // Turn LED on to show how long receiving takes.
    unsigned char *mess=LR.getMessage();
    digitalWrite(A5,LOW); // Turn off LED
    
    if(inMsgLen == 8){ // If input correct length
      if(mess[0] == 1 && mess[2] == 2 && mess[4] == 3 && mess[6] == 4){ // If registers fit correct pattern
        if(mess[1] == mess[5] && mess[3] == mess[7]){ // If the high values and low values for each 2-byte number match up (i.e. numbers the same)
          messagesReceived++;
          int8_t high = mess[1];
          int8_t low = mess[3];
          unsigned int value = (high << 8) + low;
          unsigned int diff = value - prevValue; // Unsigned means that difference will always be correct (regardless of rollover)
          if(firstMessage){
            firstMessage = false; 
          }else{
            messagesMissed += (diff-1); // -1 is because a difference of 1 means no messages in between
          }
          prevValue = value;
        }else{
          valueErrors++;
        }
      }else{
        valueErrors++;
      }
    }else{
      lengthErrors++;
    }
  }
  
  if(millis()-startTime > 10000){ // End of test
    int totalReceived = messagesReceived + valueErrors + lengthErrors;
    float missed = (messagesMissed * 100.0) / ((totalReceived+messagesMissed) * 1.0); // Percentage
    
    float correct = (messagesReceived * 100.0) / (totalReceived * 1.0); // Percentage
    float percentValueErrors = (valueErrors * 100.0) / (totalReceived * 1.0); // Percentage
    float percentLengthErrors = (lengthErrors * 100.0) / (totalReceived * 1.0); // Percentage
    
    Serial.println("Test over. The results are in: \n");
    Serial.print("Received "); Serial.print(messagesReceived); Serial.println(" perfect messages over 10 seconds.");
    Serial.print("Bitrate: "); Serial.print((messagesReceived*64)/10); Serial.println("bps");
    Serial.print("Missed "); Serial.print(missed); Serial.print("% of messages ("); Serial.print(messagesMissed); Serial.println(" messages)");
    Serial.print("Accuracy: "); Serial.print(correct); Serial.println("% of messages had no errors");
    Serial.print("--------- "); Serial.print(percentValueErrors); Serial.println("% of messages contained unexpected values");
    Serial.print("--------- "); Serial.print(percentLengthErrors); Serial.println("% of messages were of unexpected length");
  }
  
}

