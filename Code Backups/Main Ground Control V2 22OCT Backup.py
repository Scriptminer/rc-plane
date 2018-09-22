import pygame
import time
import serial
import io
from scipy.interpolate import interp1d

def mainLoop():
    ''' Repetitively Runs through the code '''
    while True:
        radio.sendData()
        inMessage = radio.readData()
        #print(inMessage)

class Joystick:
    doorOpen = '11000000'
    doorClosed = '00110000'
    doorUnlocked = '00001100'
    doorLocked = '00000011'
    prevDoor = '00000000'
    
    def __init__(self):
        pygame.init() # Initialises pygame

        self.joystick = pygame.joystick.Joystick(0) # Defines the joystick
        self.joystick.init()
        self.defaultControlRanges = [{"min":30,"centre":90,"max":150}, # Aileron
                                     {"min":30,"centre":90,"max":150}, # Elevatpr
                                     {"min":0,"centre":90,"max":180}, # Throttle
                                     {"min":30,"centre":90,"max":150}, # Rudder
                                     ] # centre allows for trim
        self.controlRanges = self.defaultControlRanges
        
        self.numaxes = int(self.joystick.get_numaxes())
        self.numbuttons = int(self.joystick.get_numbuttons())
    
    def values(self):
        ''' Returns the Joystick's values '''
        for event in pygame.event.get():
            pass
        
        # Get Axis Data
        
        axisDataList = [] 
        for i in range(self.numaxes):
            #axis = round((self.joystick.get_axis(i)+1)*89.5) # Rounds the joystick axis converted to a 0-2 range * half of 179
            r = self.controlRanges[i]; # Current control range dict
            mapper = interp1d([-1,0,1],[r["min"],r["centre"],r["max"]]) # Map function
            axis = mapper(self.joystick.get_axis(i)) # Calls the map function
            axisDataList.append(int(axis))
        
        # Get Button Data
        buttonDataList = []
        for i in range(self.numbuttons):
            button = self.joystick.get_button(i)
            buttonDataList.append(button)
        
        # Gets door values
        if buttonDataList[2] == 1: # If door open button held
            doorPos = int(self.doorOpen,2) # Open door
        else: # If released
            doorPos = int(self.doorClosed,2) # Close door
            
        if buttonDataList[3] == 1: # If door lock button held
            lockPos = int(self.doorUnlocked,2) # Unlock door
        else: # If released
            lockPos = int(self.doorLocked,2) # Lock door
        
        doorValue = doorPos+lockPos
        
        # Trim Buttons
        if buttonDataList[4] == 1: # Aileron Trim
            self.trim(0)
        elif buttonDataList[5] == 1: # Elevator Trim
            self.trim(1)
        elif buttonDataList[6] == 1: # Rudder Trim
            self.trim(3)
        elif buttonDataList[7] == 1: # Reset all trims
            self.controlRanges = self.defaultControlRanges
            print("RESET")
        
        dataArray = []
        # Appends register, then value to array
        dataArray.extend((1, axisDataList[0])) # Aileron, reg 1
        dataArray.extend((2, axisDataList[1])) # Elevator, reg 2
        dataArray.extend((3, axisDataList[3])) # Rudder, reg 3
        dataArray.extend((4, 179-axisDataList[2])) # Throttle, reg 4 (inverts)
        
        if self.prevDoor != doorValue: # Only sends if this isn't the same signal as last time
            dataArray.extend((5, doorValue)) # Drop door, reg 5
        
        self.prevDoor = doorValue
        return dataArray

    def trim(self,surfaceID):
        r = self.defaultControlRanges[surfaceID] # Gets the default control range
        mapper = interp1d([-1,0,1],[r["min"],r["centre"],r["max"]]) # Gets the numbers as if there was no trim
        currentPos = int(mapper(self.joystick.get_axis(surfaceID)))
        self.controlRanges[surfaceID]["centre"] = currentPos # Sets the centre as current position
        print(self.controlRanges[surfaceID])
        print(currentPos)
        print(r)

class Radio:
    def __init__(self):
        self.ser = serial.Serial(
            port='/dev/ttyACM0',
            rtscts = True,
            baudrate = 38400,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            writeTimeout = 1,
            timeout = 2,
        )

        self.eol = chr(0)
        self.charOffset = 1 # Adds one to each value

    def sendData(self):
        ''' Sends control data to the UNO '''
        dataToSend = joystick.values() # + whatever other things need to be added
        message = bytes([x+self.charOffset for x in dataToSend]+[ord(self.eol)]) # Adds charOffset to each character in list, and adds eol
        
        self.ser.write(message)

    def readData(self):
        if self.ser.inWaiting(): # Aborts read if there are no messages
            inMessage = self.myReadline()
            output = [byte-self.charOffset for byte in inMessage] # Takes away the charOffset
            return output
        else:
            return []

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


class OnlineInterface:
    def initialise():
        ''' Initialises the web interface '''

    def serveWebsite():
        ''' Serves the website to anyone connecting '''
        #telemetry = radio.readSerialBuffer()


# INITIALISE CODE
joystick = Joystick()
radio = Radio();
mainLoop()
