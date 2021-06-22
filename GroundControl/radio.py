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
            timeout = 0.05,
        )
        
        self.radioBuffer = [] # Buffer for data to be sent to the plane (accessed by other classes)
    
    # sendData() Not currently in use:
    #def sendData(self, dataToSend):
    #    ''' Sends control data to the UNO '''
    #    #dataToSend = joystick.values() + self.radioBuffer # + whatever other things need to be added
    #    message = bytes(dataToSend) # Adds charOffset to each character in list, and adds eol
        
    #    self.ser.write(message)
    inMessage = []
    
    def readData(self):
        char = None
        prevChar = None
        while self.ser.in_waiting: # Loop until two consecutive 255s
            prevChar = char
            char = self.ser.read()
            self.inMessage.append(char)
            
            if char == prevChar and ord(char) == 255:
                msg = self.inMessage[:-2] # Don't return the 255s (255s serve as EOL)
                
                print("Msg:")
                print(self.inMessage)
                self.inMessage = []
                return msg
        
        return []
        
        
        

