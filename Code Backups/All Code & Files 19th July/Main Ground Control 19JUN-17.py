import pygame
import time
import spidev

def mainLoop():
    ''' Repetitively Runs through the code '''
    while True:
        axisData = Radio.sendData()
        print("Aileron: "+ str(axisData["aileron"]) +", Elevator: "+ str(axisData["elevator"]) +", Throttle: "+ str(axisData["throttle"]) +", Rudder: "+ str(axisData["rudder"]))

class Joystick:
    def initialise():
        ''' Does the necessary joystick initilisation '''
        pygame.init() # Initialises pygame

        global joystick
        joystick = pygame.joystick.Joystick(0) # Defines the joystick
        joystick.init()

        global numaxes
        numaxes = str(joystick.get_numaxes())
    
    def readJoystick():
        ''' Returns the Joystick's values '''
        for event in pygame.event.get():
            pass
        
        # Get Axis Data
        axisDataList = [] 
        for i in range(int(numaxes)):
            axis = round((joystick.get_axis(i)+1)*89.5) # Rounds the joystick axis converted to a 0-2 range * half of 179
            axisDataList.append(axis)

        axisData = {}
        axisData["aileron"] = axisDataList[0]
        axisData["elevator"] = axisDataList[1]
        axisData["throttle"] = (axisDataList[2] * -1) + 179 # Throttle needs to be inverted
        axisData["rudder"] = axisDataList[3]
        
        return axisData

class Radio:
    spi = None
    
    def initialise(self):
        ''' Calls required functions to initialise the radio signal '''
        Radio.setupSPI()
        Radio.configureChip()
        
    def setupSPI(self):
        ''' Sets up the SPI connection '''
        Radio.spi = spidev.SpiDev()
        Radio.spi.open(bus, device)

    def configureChip():
        ''' Calls registers to set up the radio chip '''
        
    def rawRead(register):
        ''' Returns the value of the requested register '''
        value = 0
        return value

    def rawWrite(register,value):
        ''' Writes "value" to a specified register '''

    def readRadioBuffer():
        ''' Converts to intergers and then returns the radio chip's internal radio buffer '''
        radioBuffer = []
        return radioBuffer

    def sendData():
        ''' Sends control data to the plane '''
        joystickPositions = Joystick.readJoystick()
        # Converts the int list into a series of bytes and registers
        # Sends the necessary register positions to send data
        return joystickPositions

class OnlineInterface:
    def initialise():
        ''' Initialises the web interface '''

    def serveWebsite():
        ''' Serves the website to anyone connecting '''
        telemetry = Radio.readRadioBuffer()


# INITIALISE CODE
Joystick.initialise()
mainLoop()
