import time

from joystick import Joystick
from radio import Radio
from chatConnection import ChatConnection, startChatConnection
from managedata import ManageData

prevLoopTime = time.time()
avgLoopTime = 0

# mainLoop is called from chatConnection.py on a timer (via Tornado's event loop).
def mainLoop():
    ''' Repeatedly runs through the code '''
    
    radio.sendData(joystick.values() + radio.radioBuffer) # Sends data to plane
    #manageData.handleDownData(radio.readData())
    downData = radio.readData()
    manageData.handleDownData(downData)
    #print("Data from UNO: ")
    #print(downData)
    
    # Handles loop time
    global prevLoopTime
    global avgLoopTime
    timeDif = time.time() - prevLoopTime
    prevLoopTime = time.time()
    avgLoopTime = (timeDif+(avgLoopTime*63))/64
    print("Timedif")
    print(timeDif)
    
    currentTime = time.time() - startTime
    manageData.handlePiLoopData(avgLoopTime,currentTime)
    
    manageData.sendData() # Sends Website Data

# INITIALISES CODE
joystick = Joystick()
radio = Radio('/dev/ttyACM0')
manageData = ManageData(joystick)

startTime = time.time()

if __name__ == "__main__":
    startChatConnection() # Does not return until chat closes!
