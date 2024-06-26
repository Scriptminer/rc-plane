
class DataManager {
  public:
    DataManager (char *data_buffer, int data_buffer_length) : dataBuffer(data_buffer), dataBufferLength(data_buffer_length) {
    }
    
    void addData(char reg,char val){
      // Adds a char to the data buffer.
      dataBufferPos %= dataBufferLength; // Ensures dataBufferPos is never beyond the end of the buffer
      dataBuffer[dataBufferPos] = reg;
      dataBuffer[dataBufferPos+1] = val;
      dataBufferPos += 2;
    }
    
    void addData(int reg,int val){
      // Constrains and adds an integer to the data buffer.
      addData((char) reg, (char) constrain(val,0,255));
    }
    
    void addDataArray(char data[],int dataLength){
      // Adds an array of data to the data buffer.
      if(dataLength%2 != 0){ // Array must be even in length
        return;
      }
      for(int i=0;i<dataLength;i+=2){
        addData(data[i],data[i+1]);
      }
    }

    void getData(char** dataOut,int* len){
      // Sets dataOut to point to the beginning of the data buffer, and len to point to the end of the data contained within it
      *dataOut = dataBuffer;
      *len = dataBufferPos;
    }
    
    void resetDataBuffer(){
      // Resets the internal data buffer pointer
      dataBufferPos = 0; // Reset data buffer position
    }
  
  private:
    const int dataBufferLength;
    char *dataBuffer; // Defines the buffer for any outgoing data
    char dataBufferPos = 0;
};

