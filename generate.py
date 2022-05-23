#!/usr/bin/env python3
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
    (6,"requestTelemetry"),

    # Plane to Ground registers:
    (64,"reportedDropDoorState"),
    (65,"onboardLoopSpeed","Calculated hectomicroseconds (0.1ms units) each loop takes on average."),
    (66,"reportedControlState"),
    (67,"onboardRSSI","Received signal strength (as a positive value, not negative)"),
    (68,"currentBattVoltage","Voltage of battery (raw reading - processing to occur on PI)"),
    (69,"onboardCorruptedMessages"),
    (70,"onboardError",""), # See below in program for error codes
    (71,"onboardPacketReceiveRate","Number of packets received since last request"),
    
    (72,"accelerometerX_HB","High byte"), (73,"accelerometerX_LB","Low byte"),
    (74,"accelerometerY_HB"), (75,"accelerometerY_LB"),
    (76,"accelerometerZ_HB"), (77,"accelerometerZ_LB"),
    
    (78,"gyroX_HB"), (79,"gyroX_LB"),
    (80,"gyroY_HB"), (81,"gyroY_LB"),
    (82,"gyroZ_HB"), (83,"gyroZ_LB"),

    # Ground Arduino to PI registers:
    (128,"groundLoopSpeed","Milliseconds each loop takes on average."),
    (129,"groundRSSI"),
    (130,"groundRadioStarted","0 for success, 1 for failure"),
    (131,"aileronTrimPos"),
    (132,"elevatorTrimPos"),
    (133,"rudderTrimPos"),
    (134,"groundPacketSendRate","Incremented only when transmitted packet is >0 bytes."),
    
    # Test Channels:
    (192,"testChannel1"),
    (193,"testChannel2"),
    (194,"testChannel3"),
    (195,"testChannel4"),
    
    (255,"DO_NOT_USE"),
]

registersPrefix = "reg_"

# Ground to Air radio link:
controlCentreFrequency = 458.4875E6 # 458.4875 MHz, IR2030 pg69 (version of April 2021)
groundToAirTxPower = 20 # 100mW (20dBm), legal maximum for this band. (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW).
controlChannelNumber = 23 # Can be anywhere from 1-40
controlChannelSpacing = 25E3 # 25kHz
groundToAirBandwidth = 125E3 # 125kHz, aka 5 channels wide. Supported values: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3
groundToAirFrequency = controlCentreFrequency + (controlChannelNumber * controlChannelSpacing)
print("groundToAirFrequency: {0}Hz, groundToAirBandwidth: {1}:Hz (channel {2}), groundToAirTxPower: {3}dBm".format(groundToAirFrequency,groundToAirBandwidth,controlChannelNumber,groundToAirTxPower))

# Air to Ground radio link:
telemetryCentreFrequency = 434.04E6 - 25E3 # 434.04 Mhz - 25kHz channel spacing, IR2030 pg69 (version of April 2021)
airToGroundTxPower = 10 # 10mW (10dBm), legal maximum for this band. (2dBm = 1.5849mW, 10dBm = 10mW, 20dBm = 100mW).
telemetryChannelNumber = 17 # Can be anywhere from 1-30
telemetryChannelSpacing = 25E3
airToGroundBandwidth = 125E3 # 125kHz, aka 5 channels wide. Supported values: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3
airToGroundFrequency = telemetryCentreFrequency + (telemetryChannelNumber * telemetryChannelSpacing)
print("airToGroundFrequency: {0}Hz, airToGroundBandwidth: {1}:Hz (channel {2}), airToGroundTxPower: {3}dBm".format(airToGroundFrequency,airToGroundBandwidth,telemetryChannelNumber,airToGroundTxPower))

# Shared constants in the form: (constant name,constant value,[comments])
constants = [
    ("batteryVoltageReadingOffset",550),
    ("maxRadioMessageLength",32,"Bytes"),
    ("unlockDoorSignal",100,"Signal to send from ground to unlock door."),
    ("lockDoorSignal",200,"Signal to send from ground to lock door."),
    ("groundToAirFrequency",groundToAirFrequency),
    ("groundToAirBandwidth",groundToAirBandwidth),
    ("airToGroundFrequency",airToGroundFrequency),
    ("airToGroundBandwidth",airToGroundBandwidth),
    ("groundToAirTxPower",groundToAirTxPower),
    ("airToGroundTxPower",airToGroundTxPower),
    ("airTelemetryInterval",500,"Milliseconds between requests for telemetry from plane."),
    ("groundTelemetryInterval",250,"Milliseconds between sending Ground-Pi telemetry."),
    
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
        if(abs(const[1]) >= 32767):
            numType = "long"
        else:
            numType = "int"
        
        writeString = "const "+numType+" "+str(const[0])+" = "+str(const[1])+";"
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
