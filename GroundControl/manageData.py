import json
import time
_currentRoll = 9999
_currentPitch = 9999
_currentYaw = 9999

class ManageData():
    dataTable = {} # Generated on request - contains a series of values (+ isWarningOn byte) *only*
    #dataLookupTable = b'' # Generated on init from file - contains a series of all datapoints in the dataTable - in the same order as the dataString
    dataTableOrder = []
    numStoredServerMessages = 10
    serverMessageLength = 128 # 4 bytes integer id, 124 bytes for message
    serverMessages = [""]*10

    msgID = 0

    def __init__(self,serverMessagesFilePath):

        self.serverMessagesFilePath = serverMessagesFilePath

        with open("static/DataTableTemplate.json","r") as dataTableTemplate:
            #print("Not yet loaded the json table", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
            jsonTable = json.load(dataTableTemplate)["table"]
            for section in jsonTable:
                #print("Loading section "+section["heading"], gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
                for element in section["elements"]:
                    linkSymbol = element.get("linkSymbol")
                    if linkSymbol == None: # Standard case - only one datapoint contained - there is only 1 id
                        self.dataTable[element["id"]] = ["--",0]
                        self.dataTableOrder.append(element["id"])
                    else: # Dual datapoint element with 2 ids on one line
                        self.dataTable[element["id"][0]] = ["--",0]
                        self.dataTable[element["id"][1]] = ["--",0]
                        self.dataTableOrder.append(element["id"][0])
                        self.dataTableOrder.append(element["id"][1])
                del section
                # gc.collect()
            del jsonTable
        # gc.collect()
        #print("Completed initialising ManageData", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)

    def handleDownData(self,data):
        ''' Handles data received from UNO '''

        if len(data) % 2 != 0:
            msg = ",".join([str(c) for c in data])
            self.addServerMessage("<span color='#b77'><i> String of incorrect length recieved from UNO: {data} </i></span>".format(data=msg))
            return

        for i in range(0,len(data),2):
            reg = data[i]
            val = data[i+1]
            #print("REG IS {0}, VAL IS {1}".format(reg,val))
            # Ground to Air registers (copy of data sent to plane):
            if reg == _setAilerons:
                self.dataTable["ailerons"][0] = val

            elif reg == _setElevator:
                self.dataTable["elevator"][0]  = val

            elif reg == _setRudder:
                self.dataTable["rudder"][0]  = val

            elif reg == _setThrottle:
                self.dataTable["throttle"][0]  = round(val/1.795)

            elif reg == _setDropDoor:
                if val == _unlockDoorSignal:
                    self.addServerMessage("Sending unlock drop door signal.")
                elif val == _lockDoorSignal:
                    self.addServerMessage("Sending lock drop door signal.")

            # Plane to ground registers:
            if reg == _currentRoll:
                self.dataTable["roll"][0] = val

            elif reg == _currentPitch:
                self.dataTable["pitch"][0] = val

            elif reg == _currentYaw:
                self.dataTable["yaw"][0] = val

            elif reg == _reportedDropDoorState:
                if val == 100:
                    self.dataTable["locked"][0] = " Unlocked" # Servo position
                    self.dataTable["door"][0] = " Open" # Once the door is open, it does not propperly shut
                elif val == 200:
                    self.dataTable["locked"][0] = " Locked" # Servo position

            elif reg == _onboardLoopSpeed:
                self.dataTable["nanoDutyCycle"][0] = round(10000/val,1) # val is avg centimicroseconds (0.1ms) per loop
                if val > 200: # 20ms per loop
                    self.toggleWarning("nanoDutyCycle",True)
                else:
                    self.toggleWarning("nanoDutyCycle",False)

            elif reg == _reportedControlState:
                if val == 1: # Emergency mode
                    if self.dataTable["autopilotMode"][0] != "NoSig": # Only sends a message if mode has changed
                        self.addServerMessage("EMERGENCY MODE ENGAGED!")
                    self.dataTable["autopilotMode"][0] = "NoSig"
                    self.toggleWarning("autopilotMode",True)

                elif val == 0: # Manual / Normal mode
                    if self.dataTable["autopilotMode"][0] != "Manual": # Only sends a message if mode has changed
                        self.addServerMessage("MANUAL FLIGHT ENGAGED (breathe a sigh of relief!)")
                    self.dataTable["autopilotMode"][0] = "Manual"
                    self.toggleWarning("autopilotMode",False)

            elif reg == _onboardRSSI:
                self.dataTable["onboardRSSI"][0] = "-"+str(val)

            elif reg == _currentBattVoltage:
                readingMultiplier = 0.0229 # Constant taken from measurement
                if val == 0:
                    correctedValue = "<{0:.2f}".format( (0+_batteryVoltageReadingOffset)*readingMultiplier )
                elif val == 255:
                    correctedValue = ">{0:.2f}".format( (255+_batteryVoltageReadingOffset)*readingMultiplier )
                else:
                    correctedValue = "{0:.2f}".format( (val + _batteryVoltageReadingOffset)*readingMultiplier )
                self.dataTable["batteryVoltage"][0] = correctedValue

            elif reg == _onboardError:
                if val == 128:
                    self.addServerMessage("Inconsistant command to change drop door positions received by plane.")
                elif val == 64:
                    self.addServerMessage("Inconsistant command to control state received by plane.")
                elif val == 32:
                    self.addServerMessage("Nano loop speed too high to send.")
                elif val == 16:
                    self.addServerMessage("Function on nano attempted to send out of range int to ground.")
                elif val == 8:
                    self.addServerMessage("Nano received message of incorrect length.")

            elif reg == _onboardPacketReceiveRate:
                if val == 255:
                    self.dataTable["onboardPacketReceiveRate"][0] = "255+"
                else:
                    self.dataTable["onboardPacketReceiveRate"][0] = val

                if val < 5:
                    self.toggleWarning("onboardPacketReceiveRate",True)
                else:
                    self.toggleWarning("onboardPacketReceiveRate",False)

            # Ground to PI Registers
            elif reg == _groundLoopSpeed:
                if val == 0:
                    continue
                self.dataTable["unoDutyCycle"][0] = round(1000/val,1)
                if val > 100: # 100ms per loop
                    self.toggleWarning("unoDutyCycle",True)
                else:
                    self.toggleWarning("unoDutyCycle",False)

            elif reg == _groundRSSI:
                self.dataTable["groundRSSI"][0] = "-"+str(val)

            elif reg == _groundRadioStarted:
                if val == 0:
                    self.addServerMessage("Ground radio succesfully started!")
                if val == 1:
                    self.addServerMessage("Ground radio failed to start - reset arduino.")

            elif reg == _aileronTrimPos:
                self.dataTable["aileronsTrim"][0] = val

            elif reg == _elevatorTrimPos:
                self.dataTable["elevatorTrim"][0] = val

            elif reg == _rudderTrimPos:
                self.dataTable["rudderTrim"][0] = val

            elif reg == _groundPacketSendRate:
                if val == 255:
                    self.dataTable["groundPacketSendRate"][0] = "255+"
                else:
                    self.dataTable["groundPacketSendRate"][0] = val

                if val < 10:
                    self.toggleWarning("groundPacketSendRate",True)
                else:
                    self.toggleWarning("groundPacketSendRate",False)

            # Testing registers:
            elif reg == _testChannel1:
                print("Test channel 1: {}".format(val))

    def handlePiLoopData(self,avgLoopTime,currentTime,ramUsage):
        self.dataTable["webserverDutyCycle"][0] = round(1/(avgLoopTime+0.001))
        self.dataTable["groundTime"][0] = round(currentTime,1)
        self.dataTable["webserverRAM"][0] = round(ramUsage,2)

        if self.dataTable["webserverDutyCycle"][0] < 5:
            self.toggleWarning("webserverDutyCycle",True)
        else:
            self.toggleWarning("webserverDutyCycle",False)

        if self.dataTable["webserverRAM"][0] > 90:
            self.toggleWarning("webserverRAM",True)
        else:
            self.toggleWarning("webserverRAM",False)

    def toggleWarning(self,datapoint,toggle):
        val = 0
        if toggle == True:
            val = 1
        else:
            val = 0
        self.dataTable[datapoint][1] = val

    def addServerMessage(self,rawMsg):
        id = str(self.msgID%10000)
        strID = "0"*(4-len(id)) + id
        self.serverMessages[self.msgID%self.numStoredServerMessages] = (strID+rawMsg)

        self.msgID += 1

    def getServerMessages(self):
        return self.serverMessages

    def getStatusDataLookupTable(self):
        return self.lookupTable

    def getStatusData(self):
        """ Sends all recorded data to ChatConnection to put on webpage """
        s = time.ticks_ms()
        outData = []
        for datapoint in self.dataTableOrder:
            outData.append([str(self.dataTable[datapoint][0]),str(self.dataTable[datapoint][1])]) # val, warn
            #yield json.dumps(outData)
        t = time.ticks_ms()
        outData = json.dumps(outData)
        print("Took total:{0}ms, json:{1}ms".format(time.ticks_ms()-s,time.ticks_ms()-t))
        return outData
