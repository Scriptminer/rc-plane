import serial
import time

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
    timeout = 0.01

    def readData(self):
        endTime = time.time()+self.timeout

        while self.ser.in_waiting and time.time()<endTime: # Keep reading bytes until timeout
            # Read one byte:
            self.inMessage.append(int.from_bytes(self.ser.read(),"little"))

            if len(self.inMessage) > 2:
                prevChars = self.inMessage[-3:] # Last 3 characters of message
                #print(self.inMessage)
                if prevChars[0] == 255 and prevChars[1] == 255 and prevChars[2] != 255: # A message cannot start with a 255, so this pattern marks the end of a message
                    msg = self.inMessage[:-3] # The whole message, except the last 3 characters
                    self.inMessage = self.inMessage[-1:] # Keep only the last character, the first character of the next message
                    return msg
        """while self.ser.in_waiting:
            if self.inMessageLength == 0:
                self.inMessageLength = ord(self.ser.read()) # First byte specifies length of message
                print("Expecting message of length {}".format(self.inMessageLength))

            else: # Reading through message...
                if self.bytesRead < self.inMessageLength:
                    self.inMessage.append(self.ser.read())
                    self.bytesRead += 1

            #print("inMessage:{0},bytesRead:{1},inMessageLength:{2}".format(self.inMessage,self.bytesRead,self.inMessageLength))

            if self.bytesRead >= self.inMessageLength:
                print("Yee!")
                msg = self.inMessage
                self.inMessage = []
                self.inMessageLength = 0
                self.bytesRead = 0
                return msg"""

        return []
