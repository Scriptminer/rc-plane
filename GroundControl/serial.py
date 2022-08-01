from machine import UART
import time

class Serial:
    def __init__(self,maxMessageLength):
        self.maxMessageLength = maxMessageLength
        self.connection = UART(0, 115200)
        self.connection.init(115200, bits=8, parity=0, stop=2) # 8 data bits, even parity, 2 stop bits.
        #self.connection.irq(UART.RX_ANY, priority=1, handler=self.readData, wake=machine.IDLE)

        self.timeout_ms = 50
        self.serialBuffer = [] # Buffer for data to be sent to the plane (accessed by other classes)

    inMessage = b''

    def readData(self):
        inData = self.connection.read()
        if inData == None or len(inData) > self.maxMessageLength:
            return b''

        for inByte in inData:
            self.inMessage += bytes([inByte])
            if len(self.inMessage) > 1:
                if self.inMessage[-2] == 255 and self.inMessage[-1] == 254: # If last two characters are 255,254
                    out = self.inMessage[:-2] # Return the whole message, except the last 2 characters
                    self.inMessage = b''
                    return out # Ignore any subsequent characters

                if len(self.inMessage) > self.maxMessageLength: # TEMPORARILY STILL HERE
                    print("Dumping long message.")
                    self.inMessage = b'' # Message too long, delete it to save memory

        print("Missing EOL, dumping.")
        self.inMessage = b'' # Any partial packets received (missing EOLs) are discarded.
        return b'' # If no EOL reached, return nothing, but keep partial string for next time.
