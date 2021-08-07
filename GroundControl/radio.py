import serial

class Radio:
    def __init__(self,UNOport):
        
        print("Arduino UNO will connect on port: "+ UNOport)
        
        self.ser = serial.Serial(
            port = UNOport,
            rtscts = True,
            baudrate = 38400,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            writeTimeout = 1,
            timeout = 0.01,
        )
        
        self.radioBuffer = [] # Buffer for data to be sent to the plane (accessed by other classes)
    
    # sendData() Not currently in use:
    #def sendData(self, dataToSend):
    #    ''' Sends control data to the UNO '''
    #    #dataToSend = joystick.values() + self.radioBuffer # + whatever other things need to be added
    #    message = bytes(dataToSend) # Adds charOffset to each character in list, and adds eol
        
    #    self.ser.write(message)
    inMessage = []
    bytesRead = 0
    inMessageLength = 0
    
    def readData(self):
        while self.ser.in_waiting:
            if self.inMessageLength == 0:
                self.inMessageLength = ord(self.ser.read()) # First byte specifies length of message
                print("Expecting message of length {}".format(self.inMessageLength))
            
            else: # Reading through message...
                if self.bytesRead < self.inMessageLength:
                    self.inMessage.append(self.ser.read())
                    self.bytesRead += 1
        
        if self.bytesRead == self.inMessageLength:
            msg = self.inMessage
            self.inMessage = []
            self.inMessageLength = 0
            self.bytesRead = 0
            return msg
            
        return []
        
        
        

