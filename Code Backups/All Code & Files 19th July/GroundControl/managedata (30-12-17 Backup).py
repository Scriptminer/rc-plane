#!/usr/bin/python
# -*- coding: utf-8 -*-

class Data():
	dataTable = {} # The object used externally to look up data (table of functions)
	sensors = {"roll":{"symbol":"°","value":"---"},"pitch":{"symbol":"°","value":"---"},"yaw":{"symbol":"°","value":"---"},"speed":{"symbol":"%","value":"---"}} # Where data from sensors is stored
	
	def __init__(self):
		surfaces = {"ailerons":0,"elevator":1,"rudder":2,"throttle":3}
		for surface in surfaces:
			def getInput():
				index = surfaces[surface]
				print(index)
				return str(self.getJoystickPos()[index]) + "°"
							
			self.dataTable[surface+"Input"] = getInput
		
		'''surfaces = {"ailerons":0,"elevator":1,"rudder":3,"throttle":2}
		
		for i in surfaces:
			self.dataTable[i+"Trim"] = str(self.getTrimData()[surfaces[i]]["centre"]) + "°"
			self.dataTable[i+"Range"] = str(self.getTrimData()[surfaces[i]]["min"]) + "° - " + str(self.getTrimData()[surfaces[i]]["max"]) + "°"
		
		for sensor in self.sensors: # Cycles through all sensors
			self.dataTable[sensor] = str(getSensorVals()[sensor]["value"]) + sensors[sensor]["symbol"]
		'''
	def handleUpData(self,data): # Handles data inputed by a device
		if self.dataTable[data]["type"] == "LED":
			self.dataTable[data] = not self.dataTable[data] # Inverts the value of the register
	
	def handleStatusData(self,data):
		for i in data:
			self.dataTable[i] = data[i]
	
	def getJoystickPos(self):
		try:
			joystick.prevValues
		except:
			return
		
		return joystick.prevValues
	
	def getTrimData(self):
		try:
			joystick.controlRanges
		except:
			return
		
		return joystick.controlRanges
	
	def getSensorVals(self):
		return self.sensorSymbols
