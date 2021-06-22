# Generate CommonConstants.h and CommonConstants.py to share registers between Pi and onboard Arduino
# Also copy the master Radio.h and TelemetryManager.h into the arduino file folders.

import os

##
os.system("cp ArduinoFiles/Radio.h ArduinoFiles/RC_Full_Ground_Control/Radio.h")
os.system("cp ArduinoFiles/Radio.h ArduinoFiles/RC_Plane_Onboard_Control/Radio.h")
os.system("cp ArduinoFiles/DataManager.h ArduinoFiles/RC_Full_Ground_Control/DataManager.h")
os.system("cp ArduinoFiles/DataManager.h ArduinoFiles/RC_Plane_Onboard_Control/DataManager.h")


##

hFile = "ArduinoFiles/RC_Plane_Onboard_Control/CommonConstants.h"
pyFile = "GroundControl/CommonConstants.py"

# All registers in the form: (register number,register name,[comments])
registers = [
    # Ground to Plane registers:
    (0,"setAilerons"),
    (1,"setElevator"),
    (2,"setRudder"),
    (3,"setThrottle"),
    (4,"setDropDoor"),
    (5,"setAutopilotState"),

    # Plane to Ground registers:
    (64,"currentRoll"),
    (65,"currentPitch"),
    (66,"currentYaw"),
    (67,"reportedDropDoorState"),
    (68,"onboardLoopSpeed","Calculated execution loops per second"),
    (69,"reportedControlState"),
    (70,"onboardRSSI","Received signal strength (as a positive value, not negative)"),
    (71,"currentBattVoltage","Voltage of battery (raw reading - processing to occur on PI)"),
    (72,"onboardCorruptedMessages"),
    (73,"onboardError",""), # See below in program for error codes

    # Ground Arduino to PI registers:
    (128,"groundLoopSpeed"),
    (129,"groundRSSI"),
    (130,"groundRadioStarted","2 for success, 1 for failure"),
    (131,"aileronTrimPos"),
    (132,"elevatorTrimPos"),
    (133,"rudderTrimPos"),
]

registersPrefix = "reg_"

# Shared constants in the form: (constant name,constant value,[comments])
constants = [
    ("batteryVoltageReadingOffset",600,"Provisional number!!"),
    ("maxRadioMessageLength",32,"Bytes"),
    ("unlockDoorSignal",100,"Signal to send from ground to unlock door."),
    ("lockDoorSignal",200,"Signal to send from ground to lock door."),
    
    # Error message constants:
    ("conflictingDropDoorMessageERR",0,"Two messages sent to the drop door were both different"),
    ("conflictingControlStateMessageERR",1,"Two messages sent to change the control state were both different"),
]

with open(hFile,"w") as h_file:
    h_file.write("// This is an autogenerated file, see generate.py for edits. //\n\n")
    for reg in registers:
        writeString = "#define "+registersPrefix+str(reg[1])+" "+str(reg[0])
        if len(reg) == 3: # i.e. if comment exists
            writeString += " // "+reg[2]
        h_file.write(writeString+"\n")

    h_file.write("\n")

    for const in constants:
        writeString = "const int "+str(const[0])+" = "+str(const[1])+";"
        if len(const) == 3: # i.e. if comment exists
            writeString += " // "+const[2]
        h_file.write(writeString+"\n")

with open(pyFile,"w") as py_file:
    py_file.write("## This is an autogenerated file, see generate.py for edits. ##\n\n")
    py_file.write("registers = {\n")
    for reg in registers:
        writeString = '    "'+str(reg[1])+'": '+str(reg[0])+','
        if len(reg) == 3: # i.e. if comment exists
            writeString += "   # "+reg[2]
        py_file.write(writeString+"\n")
    py_file.write("}\n\n")

    py_file.write("commonConstants = {\n")
    for const in constants:
        writeString = '    "'+str(const[0])+'": '+str(const[1])+','
        if len(reg) == 3: # i.e. if comment exists
            writeString += "   # "+const[2]
        py_file.write(writeString+"\n")

    py_file.write("}\n")



os.system("cp ArduinoFiles/RC_Plane_Onboard_Control/CommonConstants.h ArduinoFiles/RC_Full_Ground_Control/CommonConstants.h")
