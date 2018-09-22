import pygame
import time
import serial
import io
from scipy.interpolate import interp1d
from copy import deepcopy

def mainLoop():
    ''' Repetitively Runs through the code '''
    while True:
        radio.sendData()
        #inMessage = radio.readData()
        #print(inMessage)

class Joystick:
    prevDoor = 200
    
    def __init__(self):
        pygame.init() # Initialises pygame

        self.joystick = pygame.joystick.Joystick(0) # Defines the joystick
        self.joystick.init()
        self.defaultControlRanges = [{"min":30,"centre":90,"max":150}, # Aileron
                                     {"min":0,"centre":90,"max":180}, # Elevator
                                     {"min":0,"centre":90,"max":180}, # Throttle
                                     {"min":30,"centre":90,"max":150}, # Rudder
                                     ] # centre allows for trim
        self.controlRanges = deepcopy(self.defaultControlRanges)
        
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
            doorPos = 100 # Open door
        else: # If released
            doorPos = 200 # Close door
        
        # Trim Buttons
        if buttonDataList[4] == 1: # Aileron Trim
            self.trim(0)
        elif buttonDataList[5] == 1: # Elevator Trim
            self.trim(1)
        elif buttonDataList[6] == 1: # Rudder Trim
            self.trim(3)
        elif buttonDataList[7] == 1: # Reset all trims
            self.controlRanges = self.defaultControlRanges
        
        dataArray = []
        # Appends register, then value to array
        dataArray.extend((1, axisDataList[0])) # Aileron, reg 1
        dataArray.extend((2, axisDataList[1])) # Elevator, reg 2
        dataArray.extend((3, axisDataList[3])) # Rudder, reg 3
        dataArray.extend((4, 179-axisDataList[2])) # Throttle, reg 4 (inverts)
        
        if self.prevDoor != doorPos: # Only sends if this isn't the same signal as last time
            dataArray.extend((5, doorPos)) # Drop door, reg 5
        
        self.prevDoor = doorPos
        return dataArray

    def trim(self,surfaceID):
        r = self.defaultControlRanges[surfaceID] # Gets the default control range
        mapper = interp1d([-1,0,1],[r["min"],r["centre"],r["max"]]) # Gets the numbers as if there was no trim
        currentPos = int(mapper(self.joystick.get_axis(surfaceID)))
        self.controlRanges[surfaceID]["centre"] = currentPos # Sets the centre as current position
        
class Radio:
    def __init__(self):
        self.ser = serial.Serial(
            port='/dev/ttyACM0',
            rtscts = True,
            baudrate = 38400,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            writeTimeout = 5,
            timeout = 2,
        )

        self.eol = chr(0)
        self.charOffset = 1 # Adds one to each value

    def sendData(self):
        ''' Sends control data to the UNO '''
        #dataToSend = joystick.values() # + whatever other things need to be added
        #message = bytes([x+self.charOffset for x in dataToSend]+[ord(self.eol)]) # Adds charOffset to each character in list, and adds eol
        
        dataToSend = joystick.values()
        message = ""
        for num in dataToSend:
            message += str(num)
            message += ","
        print(message)
        self.ser.write(bytes(message,"ascii"))

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
