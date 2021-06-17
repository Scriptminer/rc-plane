
class RADIO {
  public:
    RADIO (int max_radio_msg) : maxRadioMsg(max_radio_msg) {
    }
  
    bool begin(unsigned long frequency, int txPower){
      // Begin LoRa:
      if (!LoRa.begin(frequency)) {
        return false;
      }else{
        LoRa.setTxPower(constrain(txPower, 2, 20)); // txPower must be between 2 and 20 
        return true;
      }
    }
    
    int receiveData(byte inDataBuffer[]){
      // Write any incoming messages to inDataBuffer, and return the length of the message
      
      int packetSize = LoRa.parsePacket();
      
      if(packetSize > 0 && packetSize <= maxRadioMsg){
        lastSignal = micros();
        int i = 0;
        while (LoRa.available() && i <= maxRadioMsg) {
          byte inByte = LoRa.read();
          inDataBuffer[i] = inByte;
          i++;
        }
        
        int avgRSSI = (abs(LoRa.packetRssi())+avgRSSI) / 2; // Rolling average
        
        return packetSize;
      }
    }

    void transmitData(byte txData[], int len){
      LoRa.beginPacket();
      for(int i=0;i<len;i++){
        LoRa.print(txData[i]);
      }
      LoRa.endPacket();
    }

    int getAvgRSSI(){ return avgRSSI; }
    unsigned long getLastSignal(){ return lastSignal; }

  private:
    const int maxRadioMsg;
    int avgRSSI;
    unsigned long lastSignal; // Time of last signal
};

class SERIAL_IMITATION_RADIO {
  public:
    SERIAL_IMITATION_RADIO (int max_radio_msg) : maxRadioMsg(max_radio_msg) {
    }
  
    bool begin(unsigned long frequency, int txPower){
      // Begin "radio":
      Serial.begin(38400);
      return true;
    }
    
    int receiveData(byte inDataBuffer[]){
      // Write any incoming messages to inDataBuffer, and return the length of the message
      
      int packetSize = Serial.available();
      
      if(packetSize > 0){
        lastSignal = micros();
        int i = 0;
        while (Serial.available() && i <= maxRadioMsg) {
          byte inByte = Serial.parseInt();
          Serial.print("I received: ");Serial.print((int) inByte);Serial.println();
          inDataBuffer[i] = inByte;
          i++;
        }
        
        int avgRSSI = (abs(-68)+avgRSSI) / 2; // Rolling average
        
        return packetSize;
      }
    }

    void transmitData(byte txData[], int len){
      for(int i=0;i<len;i++){
        Serial.print((int) txData[i]); Serial.print(",");
      }
      Serial.println();
    }

    int getAvgRSSI(){ return avgRSSI; }
    unsigned long getLastSignal(){ return lastSignal; }

  private:
    const int maxRadioMsg;
    int avgRSSI;
    unsigned long lastSignal; // Time of last signal
};


