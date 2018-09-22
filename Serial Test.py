import time
import serial

def setup():
    global ser
    ser = serial.Serial(
        port='/dev/ttyACM0',
        baudrate = 38400,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
    )
    #receive()

def receive(numChars = 100):
    inData = ser.read(numChars)
    for i in inData:
        print(i)

def send(data):
    #time.sleep(10)
    '''msg = b''
    for i in data:
        msg'''
    ser.write(data)

setup()
