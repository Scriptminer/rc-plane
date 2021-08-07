
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

class SENSOR_MANAGER {
  public:
    SENSOR_MANAGER (){
    }
    
    int calculateRPY(){
      // returns: Roll, Pitch, Yaw
      
    }

    void updateReadings(){
      avgBatteryVoltageReading = ( (avgBatteryVoltageReading*(8-1)) + analogRead(A6)) / 8;
    }
    
    int getBatteryVoltage(){
//      Serial.println(avgBatteryVoltageReading);
      return avgBatteryVoltageReading - batteryVoltageReadingOffset;
    }
    
  private:    
    int avgBatteryVoltageReading = 0;
    
    int returnMPU6050(){
      // returns: accellerometer, gyros
      
    }  
};


