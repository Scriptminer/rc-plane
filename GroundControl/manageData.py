from chatConnection import ChatConnection
from CommonConstants import registers
from copy import deepcopy

class ManageData():
    dataTable = {
        "roll":{"symbol":"°","value":"---"},
        "pitch":{"symbol":"°","value":"---"},
        "yaw":{"symbol":"°","value":"---"},
        "speed":{"symbol":"mph","value":"---"},
        
        "locked":{"symbol":"","value":"Locked"},
        "door":{"symbol":"","value":"Closed"},
        "doorSafety":{"symbol":"","value":"---"},
        
        "piLoopSpeed":{"symbol":"/s","value":"---"},
        "nanoLoopSpeed":{"symbol":"/s","value":"---"},
        "unoLoopSpeed":{"symbol":"/s","value":"---"},
        "groundTime":{"symbol":"s","value":"---"},
        "autopilotMode":{"symbol":"","value":"---"},
        
        "voltage":{"symbol":"v","value":"---"},
        "amps":{"symbol":"A","value":"---"},
        "batteryTemperature":{"symbol":"°C","value":"---"},
        
        "altitude":{"symbol":"m","value":"---"},
        "outsideTemperature":{"symbol":"°C","value":"---"},
        "airspeed":{"symbol":"mph","value":"---"},
        "gforce":{"symbol":"Gs","value":"---"},
        
        "frontLight":{"symbol":"","value":False},
        "wingSolid":{"symbol":"","value":False},
        "wingStrobe":{"symbol":"","value":False},
        "tailStrobe":{"symbol":"","value":False},
        "dropLight":{"symbol":"","value":False},
        "tailNavs":{"symbol":"","value":False},
        "bodyLights":{"symbol":"","value":False},
                }
    
    warningsToSend = [] # Example entry {"element":"autopilotMode","toggle":True}
    
    def __init__(self):
        surfaces = [["ailerons","°"],["elevator","°"],["rudder","°"],["throttle","%"]]
        for surface in surfaces:
            self.dataTable[surface[0]+"Input"] = {"symbol":surface[1],"value":"---"}
            self.dataTable[surface[0]+"Trim"] = {"symbol":surface[1],"value":"---"}
            #self.dataTable[surface[0]+"Range"] = {"symbol":surface[1],"value":"---"}
    
    # handleUpData() not currently in use
    """def handleUpData(self,data):
        ''' Handle data inputed by a device '''
        if self.dataTable[data]["type"] == "LED":
            self.dataTable[data]["value"] = self.dataTable[data]["value"] # Inverts the value of the register
        
        radio.radioBuffer.extend(()) # Will ultimately send data to the UNO to change LEDS onboard
    
        return joystick.prevValues"""
    
    
    def handleDownData(self,data):
        ''' Handles data received from UNO '''

        if len(data) % 2 != 0:
            ChatConnection.send_all_msg("String of incorrect length recieved from UNO")
            print("String of incorrect length recieved from UNO, of length: ")
            print(len(data))
            return
                
        for i in range(0,len(data),2):
            reg = ord(data[i])
            val = ord(data[i+1])
            print("REG IS {0}, VAL IS {1}".format(reg,val))
            # Ground to Air registers (copy of data sent to plane):
            if reg == registers["setAilerons"]:
                self.dataTable["aileronsInput"]["value"] = val 
                
            elif reg == registers["setElevator"]:
                self.dataTable["elevatorInput"]["value"]  = val
                
            elif reg == registers["setRudder"]:
                self.dataTable["rudderInput"]["value"]  = val
                
            elif reg == registers["setThrottle"]:
                self.dataTable["throttleInput"]["value"]  = round(val/1.795)
                
            elif reg == registers["setDropDoor"]:
                if val == unlockDoorSignal:
                    ChatConnection.send_all_msg("Sending unlock drop door signal.")
                elif val == lockDoorSignal:
                    ChatConnection.send_all_msg("Sending lock drop door signal.")
                
            # Plane to ground registers:
            if reg == registers["currentRoll"]:
                self.dataTable["roll"]["value"] = val
            
            elif reg == registers["currentPitch"]:
                self.dataTable["pitch"]["value"] = val
            
            elif reg == registers["currentYaw"]:
                self.dataTable["yaw"]["value"] = val
            
            elif reg == registers["reportedDropDoorState"]:
                if val == 100:
                    self.dataTable["locked"]["value"] = " Unlocked" # Servo position
                    self.dataTable["door"]["value"] = " Open" # Once the door is open, it does not propperly shut
                elif val == 200:
                    self.dataTable["locked"]["value"] = " Locked" # Servo position
            
            elif reg == registers["onboardLoopSpeed"]:
                if val == 255:
                    self.dataTable["nanoLoopSpeed"]["value"] = ">=255"
                else:
                    self.dataTable["nanoLoopSpeed"]["value"] = val
                
                if val < 10:
                    self.warningsToSend.append({"element":"nanoLoopSpeed","toggle":True})
                else:
                    self.warningsToSend.append({"element":"nanoLoopSpeed","toggle":False})
            
            elif reg == registers["reportedControlState"]:
                if val == 1: # Emergency mode
                    if self.dataTable["autopilotMode"]["value"] != "NoSig": # Only sends a message if mode has changed
                        ChatConnection.send_all_msg("EMERGENCY MODE ENGAGED!")
                    self.dataTable["autopilotMode"]["value"] = "NoSig"
                    self.warningsToSend.append({"element":"autopilotMode","toggle":True})
                    
                elif val == 0: # Manual / Normal mode
                    if self.dataTable["autopilotMode"]["value"] != "Manual": # Only sends a message if mode has changed
                        ChatConnection.send_all_msg("MANUAL FLIGHT ENGAGED (breathe a sigh of relief!)")
                    self.dataTable["autopilotMode"]["value"] = "Manual"
                    self.warningsToSend.append({"element":"autopilotMode","toggle":False})
            
            elif reg == registers["onboardError"]:
                if val == 128:
                    ChatConnection.send_all_msg("Inconsistant command to change drop door positions received by plane.")
                elif val == 64:
                    ChatConnection.send_all_msg("Inconsistant command to control state received by plane.")
                elif val == 32:
                    ChatConnection.send_all_msg("Nano loop speed too high to send.")
                elif val == 16:
                    ChatConnection.send_all_msg("Function on nano attempted to send out of range int to ground.")
                elif val == 8:
                    ChatConnection.send_all_msg("Nano received message of incorrect length.")
                    
            elif reg == registers["currentBattVoltage"]:
                self.dataTable["voltage"]["value"] = val
            
            # Ground to PI Registers
            elif reg == registers["groundLoopSpeed"]:
                self.dataTable["unoLoopSpeed"]["value"] = val
                if val < 10:
                    self.warningsToSend.append({"element":"unoLoopSpeed","toggle":True})
                else:
                    self.warningsToSend.append({"element":"unoLoopSpeed","toggle":False})
            
            elif reg == registers["groundRadioStarted"]:
                if val == 100:
                    ChatConnection.send_all_msg("Ground radio succesfully started!")
                if val == 200:
                    ChatConnection.send_all_msg("Ground radio failed to start - reset arduino.")
            
            elif reg == registers["aileronTrimPos"]:
                self.dataTable["aileronsTrim"]["value"] = val
            
            elif reg == registers["elevatorTrimPos"]:
                self.dataTable["elevatorTrim"]["value"] = val
            
            elif reg == registers["rudderTrimPos"]:
                self.dataTable["rudderTrim"]["value"] = val
                
    
    def handlePiLoopData(self,avgLoopTime,currentTime):
        self.dataTable["piLoopSpeed"]["value"] = round(1/avgLoopTime)
        self.dataTable["groundTime"]["value"] = round(currentTime)
        
        if self.dataTable["piLoopSpeed"]["value"] < 5:
            self.warningsToSend.append({"element":"piLoopSpeed","toggle":True})
        else:
            self.warningsToSend.append({"element":"piLoopSpeed","toggle":False})
    
    def sendData(self):
        """ Sends all recorded data to ChatConnection to put on webpage """
                
        # Sends Data
        dataToSend = {}
        for i in self.dataTable:
            dataToSend[i] = str(self.dataTable[i]["value"]) + str(self.dataTable[i]["symbol"])
        
        ChatConnection.send_all_json({"type":"warning","data":self.warningsToSend})
        ChatConnection.send_all_json({"type":"data","data":dataToSend})
        
        self.warningsToSend = []
                
