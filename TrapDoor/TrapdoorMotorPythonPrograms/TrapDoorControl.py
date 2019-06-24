#!/usr/bin/python

import serial, time, sys

class TrapDoorControl:

	def __init__(self, port):
		self.responseAddr = "o1901.3"
		self.commandAddr = "o1901.2"
		self.statusAddr = "o1901.1"
		self.fillCmd = "3"
		self.holdCmd = "1"
		self.dumpCmd = "2"
		self.catUpCmd = "4"
		self.ser = serial.Serial()
		self.ser.port=port
		self.ser.baudrate=115200
		self.ser.parity=serial.PARITY_NONE
		self.ser.stopbits=serial.STOPBITS_ONE
		self.ser.bytesize=serial.EIGHTBITS
		self.ser.xonxoff=True
		self.ser.timeout=1
		self.ser.writeTimeout=2
		self.ser.open()
		if(self.ser.isOpen() == True):
			self.ser.flushInput()
			self.ser.flushOutput()
		else:
			raise Exception("Could not establish serial interface.")
	
	def __del__(self):
		self.ser.close()

	def writeObject(self, object, value):
		self.ser.write(object+"="+value+"\r")
		self.ser.readline()

	def readObject(self, object):
		self.ser.write(object+"\r")
		time.sleep(.01)
		result = self.ser.readline()
	        print "readObject result:",result #dbg
        	return result

	def isBusy(self):
		response = self.readObject(self.statusAddr)
		if(float(response) != 0):
			return True
		else:
			return False

	def fill(self):
		self.writeObject(self.commandAddr, self.fillCmd)
		response = self.readObject(self.responseAddr)
                print "Response: ", response #dbg
		if(float(response) == 1):
			return True
		else:
			return False

	def hold(self):
		self.writeObject(self.commandAddr, self.holdCmd)
		response = self.readObject(self.responseAddr)
		if(float(response) == 1):
			return True
		else:
			return False

	def dump(self):
		self.writeObject(self.commandAddr, self.dumpCmd)
		response = self.readObject(self.responseAddr)
		if(float(response) == 1):
			return True
		else:
			return False
	
	def up(self):
		self.writeObject(self.commandAddr, self.catUpCmd)
		response = self.readObject(self.responseAddr)
		if(float(response) == 1):
			return True
		else:
			return False
