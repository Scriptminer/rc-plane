import pygame
from copy import deepcopy
from scipy.interpolate import interp1d

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
        self.defaultControlRanges = [{"min":30,"centre":90,"max":150,"typ":"ailerons"}, # Aileron
                                     {"min":0,"centre":90,"max":180,"typ":"elevator"}, # Elevator
                                     {"min":0,"centre":90,"max":180,"typ":"throttle"}, # Throttle
                                     {"min":30,"centre":90,"max":150,"typ":"rudder"}, # Rudder
                                     ] # centre allows for trim
        
        self.controlRanges = deepcopy(self.defaultControlRanges)
        
        self.prevValues = []
        
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
        if buttonDataList[2] == 1 and buttonDataList[3] == 1: # If both buttons held
            doorValue = 100
        else: # If released
            doorValue = 200
        
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
        
        if self.prevDoor != doorValue: # Only sends if this isn't the same signal as last time
            dataArray.extend((5, doorValue)) # Drop door, reg 5
            dataArray.extend((5, doorValue)) # Sent twice for confirmation
            
        self.prevDoor = doorValue
        self.prevValues = dataArray
        return dataArray

    def trim(self,surfaceID):
        r = self.defaultControlRanges[surfaceID] # Gets the default control range
        mapper = interp1d([-1,0,1],[r["min"],r["centre"],r["max"]]) # Gets the numbers as if there was no trim
        currentPos = int(mapper(self.joystick.get_axis(surfaceID)))
        self.controlRanges[surfaceID]["centre"] = currentPos # Sets the centre as current position
