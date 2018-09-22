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
        
        self.eol = chr(0)
        self.charOffset = 1 # Adds one to each value
        
        self.radioBuffer = [] # Buffer for data to be sent to the plane (accessed by other classes)
    
    def sendData(self, dataToSend):
        ''' Sends control data to the UNO '''
        #dataToSend = joystick.values() + self.radioBuffer # + whatever other things need to be added
        message = bytes( [x+self.charOffset for x in dataToSend] + [ord(self.eol)]) # Adds charOffset to each character in list, and adds eol
        
        self.ser.write(message)
    
    def readData(self):
        inMessage = self.myReadline()
        output = [byte-self.charOffset for byte in inMessage] # Takes away the charOffset
        return output
        '''if self.ser.inWaiting(): # Aborts read if there are no messages
            inMessage = self.myReadline()
            output = [byte-self.charOffset for byte in inMessage] # Takes away the charOffset
            return output
        else:
            print("nothing worthwhile....")
            return []'''
    
    def myReadline(self): # Custom EOL readline function from goo.gl/U6REhB
        eol = self.eol
        line = bytearray()
        while True:
            c = self.ser.read(1)
            if c:
                if ord(c) == ord(eol): # Ord so that the characters are in the same format
                    break
                else:
                    line += c   
            else:
                break
        
        return bytes(line)
