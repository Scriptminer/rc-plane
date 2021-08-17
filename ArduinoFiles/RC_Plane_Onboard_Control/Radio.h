
class RADIO {
  public:
    RADIO (int max_radio_msg, unsigned long rx_frequency, unsigned long tx_frequency, unsigned long rx_bandwidth, unsigned long tx_bandwidth, int tx_power) : maxRadioMsg(max_radio_msg), rxFrequency(rx_frequency), txFrequency(tx_frequency), rxBandwidth(rx_bandwidth), txBandwidth(rx_bandwidth), txPower(tx_power) {
    }
  
    bool begin() {
      // Begin LoRa:
      if (!LoRa.begin(rxFrequency)) { // RX Frequency is the default.
        return false;
      }else{
        LoRa.setTxPower(constrain(txPower, 2, 20)); // txPower must be between 2 and 20
        LoRa.setSignalBandwidth(rxBandwidth); // RX Bandwidth is the default.
        return true;
      }
    }
    
    bool receiveData(char inDataBuffer[],int* inDataLength,unsigned long waitUntilTimestamp = 0){
      // Write any incoming messages to inDataBuffer, and return the length of the message
      
      int packetSize = LoRa.parsePacket();
      while(!packetSize && millis() < waitUntilTimestamp){
        packetSize = LoRa.parsePacket();
      }
      
      if(packetSize > 0 && packetSize <= maxRadioMsg){
        lastSignal = millis();
        int i = 0;
        while (LoRa.available() && i <= maxRadioMsg) {
          char inByte = LoRa.read();
          inDataBuffer[i] = inByte;
          i++;
        }
        *inDataLength = i;
        avgRSSI = (abs(LoRa.packetRssi())+avgRSSI) / 2; // Rolling average
        return true;
      }
      return false;
    }
    

    void transmitData(char txData[], int len){
      LoRa.setFrequency(txFrequency); // Switch to transmit frequency
      LoRa.setSignalBandwidth(txBandwidth); // Switch to transmit bandwidth
      LoRa.beginPacket();
      LoRa.write(txData, len);
      LoRa.endPacket();
      LoRa.setSignalBandwidth(rxBandwidth); // Return to receive bandwidth
      LoRa.setFrequency(rxFrequency); // Return to receive frequency
    }

    int getAvgRSSI(){ return avgRSSI; }
    unsigned long getLastSignal(){ return lastSignal; }

  private:
    const int maxRadioMsg = 0;
    int avgRSSI = 0;
    unsigned long lastSignal = 0; // Time of last signal
    
    unsigned long rxFrequency = 433E6; // Default
    unsigned long txFrequency = 433E6; // Default
    unsigned long rxBandwidth = 125E3; // Default
    unsigned long txBandwidth = 125E3; // Default
    int txPower = 2; // Default
};

/*
// Currently Broken
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
        lastSignal = millis();
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
*/

