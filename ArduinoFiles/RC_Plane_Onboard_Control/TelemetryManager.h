
class TELEMETRY_MANAGER {
  public:
    TELEMETRY_MANAGER (byte *telemetry_buffer, int telemetry_buffer_length) : telemetryBuffer(telemetry_buffer), telemetryBufferLength(telemetry_buffer_length) {
    }
    
    void addTelemetry(byte reg,byte val){
      telemetryBufferPos %= telemetryBufferLength; // Ensures telemetryBufferPos is never beyond the end of the buffer
      telemetryBuffer[telemetryBufferPos] = reg;
      telemetryBuffer[telemetryBufferPos+1] = val;
      telemetryBufferPos += 2;
    }
    
    void addTelemetry(int reg,int val){
      if((val > 255) || (val < 0)){ // Outside permissable range
        addTelemetry(groundregCONFLICT,B00010000); // Generic byte too high
        return;
      }else{
        addTelemetry((byte) reg, (byte) val);
      }
    }

    int getTelemetry(byte telemetryOut[]){
      // Writes the telemetry buffer to telemetryOut, then return length of the telemetry buffer.
      telemetryOut = telemetryBuffer;
      int tmpTelemetryBufferPos = telemetryBufferPos;
      telemetryBufferPos = 0; // Reset telemetry buffer position
      return tmpTelemetryBufferPos;
    }
  
  private:
    const int telemetryBufferLength;
    byte *telemetryBuffer; // Defines the buffer for any outgoing telemetry
    byte telemetryBufferPos = 0;
};

