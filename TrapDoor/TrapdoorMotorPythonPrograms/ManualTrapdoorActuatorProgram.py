#!/usr/bin/python

from TrapDoorControl import *
import serial, time, sys

trapdoorControl = TrapDoorControl('../../ports/ttyftdiA603J6ZY')

while(True):
	input = raw_input("Please make a selection: (\033[1mH\033[0m)old, (\033[1mF\033[0m)ill, (\033[1mD\033[0m)ump, \033[1mU\033[0m)p, or (\033[1mQ\033[0m)uit\n")
	if(input == "H"):
		response = trapdoorControl.hold()
		if(float(response) == 1):
			print "Told motor to Hold. Response: "+str(response)+" (Command Accepted)"
		else:
			print "Error! Told motor to Hold. Response: "+str(response)+" (Command Not Accepted)"
	elif(input == "F"):
		response = trapdoorControl.fill()
		if(float(response) == 1):
			print "Told motor to Fill. Response: "+str(response)+" (Command Accepted)"
		else:
			print "Error! Told motor to Fill. Response: "+str(response)+" (Command Not Accepted)"
	elif(input == "D"):
		response = trapdoorControl.dump()
		if(float(response) == 1):
			print "Told motor to Dump. Response: "+str(response)+" (Command Accepted)"
		else:
			print "Error! Told motor to Dump. Response: "+str(response)+" (Command Not Accepted)"
	elif(input == "U"):
		response = trapdoorControl.up()
		if(float(response) == 1):
			print "Told motor to Up. Response: "+str(response)+" (Command Accepted)"
		else:
			print "Error! Told motor to Up. Response: "+str(response)+" (Command Not Accepted)"
	elif(input == "Q"):
		break
	else:
		print "Error! Unknown command. Please enter a known command."
	
	while(trapdoorControl.isBusy() == True):
		print "Waiting for motor to stop executing command."
		time.sleep(1)
	
	print "Command Finished. Time taken to execute: Flapping: "+trapdoorControl.readObject("o1901.5")+" Shaft: "+trapdoorControl.readObject("o1901.6")+"."

print "Thank you for using your mom's program!"
