from chatConnection import ChatConnection
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
    
    def __init__(self,joystick):
        self.joystick = joystick
        surfaces = [["ailerons","°"],["elevator","°"],["rudder","°"],["throttle","%"]]
        for surface in surfaces:
            self.dataTable[surface[0]+"Input"] = {"symbol":surface[1],"value":"---"}
            self.dataTable[surface[0]+"Trim"] = {"symbol":surface[1],"value":"---"}
            self.dataTable[surface[0]+"Range"] = {"symbol":surface[1],"value":"---"}
    
    def handleUpData(self,data):
        ''' Handle data inputed by a device '''
        if self.dataTable[data]["type"] == "LED":
            self.dataTable[data]["value"] = self.dataTable[data]["value"] # Inverts the value of the register
        
        radio.radioBuffer.extend(()) # Will ultimately send data to the UNO to change LEDS onboard
    
        return joystick.prevValues
    
    
    def handleDownData(self,data):
        ''' Handles data received from UNO '''
        # Arduino down registers:
        '''int groundregPING = 1;
           int groundregROLL = 2;
           int groundregPITCH = 3;
           int groundregYAW = 4;
           int groundregDROP = 5; // Returns drop door state
           int groundregLOOP = 6; // Loop time register (avg loop)
           int groundregAUTOMODE = 7; // Autopilot state
           int groundregCONFLICT = 8; // Conflicting incoming signal warning. #bit1 Drop door, #bit2 control state toggle, #bit3 out of range int submitted
        '''
        if len(data) % 2 != 0:
            ChatConnection.send_all_msg("String of incorrect length recieved from UNO")
            print("String of incorrect length recieved from UNO, of length: ")
            print(len(data))
            return
        
        for i in range(0,len(data),2):
            reg = data[i]
            val = data[i+1]
            if reg == 1: # Ping
                ''' Nothing as of yet '''
            
            elif reg == 2: # Roll
                self.dataTable["roll"]["value"] = val
            
            elif reg == 3: # Pitch
                self.dataTable["pitch"]["value"] = val
            
            elif reg == 4: # Yaw
                self.dataTable["yaw"]["value"] = val
            
            elif reg == 5: # Drop
                if val == 100:
                    self.dataTable["locked"]["value"] = " Unlocked" # Servo position
                    self.dataTable["door"]["value"] = " Open" # Once the door is open, it does not propperly shut
                elif val == 200:
                    self.dataTable["locked"]["value"] = " Locked" # Servo position
            
            elif reg == 6: # Loop
                if val == 254:
                    self.dataTable["nanoLoopSpeed"]["value"] = ">2540"
                else:
                    self.dataTable["nanoLoopSpeed"]["value"] = val*10
                
                if val*10 < 10:
                    self.warningsToSend.append({"element":"nanoLoopSpeed","toggle":True})
                else:
                    self.warningsToSend.append({"element":"nanoLoopSpeed","toggle":False})
            
            elif reg == 7: # Autopilot mode
                if val == 240: # First 4 bits high, emergency mode
                    if self.dataTable["autopilotMode"]["value"] != "NoSig": # Only sends a message if mode has changed
                        ChatConnection.send_all_msg("EMERGENCY MODE ENGAGED!")
                    self.dataTable["autopilotMode"]["value"] = "NoSig"
                    self.warningsToSend.append({"element":"autopilotMode","toggle":True})
                    
                elif val == 15: # Second 4 bits high, normal mode
                    if self.dataTable["autopilotMode"]["value"] != "Manual": # Only sends a message if mode has changed
                        ChatConnection.send_all_msg("MANUAL FLIGHT ENGAGED (breathe a sigh of relief!)")
                    self.dataTable["autopilotMode"]["value"] = "Manual"
                    self.warningsToSend.append({"element":"autopilotMode","toggle":False})
            
            elif reg == 8: # Conflict
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
            
            elif reg == 9: # Testing
                print("Signal to reg 9 is")
                print(val)
            
            # From UNO Registers
            elif reg == 128: # UNO loop speed
                self.dataTable["unoLoopSpeed"]["value"] = val
                if val < 10:
                    self.warningsToSend.append({"element":"unoLoopSpeed","toggle":True})
                else:
                    self.warningsToSend.append({"element":"unoLoopSpeed","toggle":False})
            
            elif reg == 129: # Radio begin
                if val == 100:
                    ChatConnection.send_all_msg("Radio succesfully started!")
                if val == 200:
                    ChatConnection.send_all_msg("Radio failed to start - reset arduino.")
    
    def handleJoystickData(self):
        """ Takes data from joystick """
        # Trim
        controlRanges = deepcopy(self.joystick.controlRanges)
        for i in controlRanges[2]:
            if i != "typ":
                controlRanges[2][i] = round(controlRanges[2][i]/1.795)
        
        for i in controlRanges:
            trim = self.dataTable[i["typ"]+"Trim"]
            ctrlRange = self.dataTable[i["typ"]+"Range"]
            
            trim["value"] = str(i["centre"])
            ctrlRange["value"] = str(i["min"]) + ctrlRange["symbol"] + "-" + str(i["max"]) # Symbol is added on send
        
        # Inputs
        data = self.joystick.prevValues
        for x in range(0,len(data),2):
            reg = data[x]
            val = data[x+1]
            if reg == 1: # Ailerons
                self.dataTable["aileronsInput"]["value"] = val 
            elif reg == 2: # Elevator
                self.dataTable["elevatorInput"]["value"]  = val
            elif reg == 3: # Rudder
                self.dataTable["rudderInput"]["value"]  = val
            elif reg == 4: # Throttle
                self.dataTable["throttleInput"]["value"]  = round(val/1.795)
            elif reg == 5: # Drop Door
                pass
                # Only displayed when recieving from plane
    
    def handlePiLoopData(self,avgLoopTime,currentTime):
        self.dataTable["piLoopSpeed"]["value"] = round(1/avgLoopTime)
        self.dataTable["groundTime"]["value"] = round(currentTime)
        
        if self.dataTable["piLoopSpeed"]["value"] < 5:
            self.warningsToSend.append({"element":"piLoopSpeed","toggle":True})
        else:
            self.warningsToSend.append({"element":"piLoopSpeed","toggle":False})
    
    def sendData(self):
        # Handles extra data
        
        self.handleJoystickData()
                
        # Sends Data
        dataToSend = {}
        for i in self.dataTable:
            dataToSend[i] = str(self.dataTable[i]["value"]) + str(self.dataTable[i]["symbol"])
        
        ChatConnection.send_all_json({"type":"warning","data":self.warningsToSend})
        ChatConnection.send_all_json({"type":"data","data":dataToSend})
        
        self.warningsToSend = []
                
