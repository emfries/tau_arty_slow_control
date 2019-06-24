#!/usr/bin/python

import serial, time, sys

def writeObject(ser, object, value):
	ser.write(object+"="+value+"\r")
	time.sleep(.05)
	ser.readline()

def readObject(ser, object):
	ser.write(object+"\r")
	time.sleep(.05)
	result = ser.readline()
	return result


ser = serial.Serial()
ser.port='/dev/trapdoor'
ser.baudrate=115200
ser.parity=serial.PARITY_NONE
ser.stopbits=serial.STOPBITS_ONE
ser.bytesize=serial.EIGHTBITS
ser.xonxoff=True
ser.timeout=1
ser.writeTimeout=2
ser.open()
if(ser.isOpen() == True):
	print "Successfully opened serial port."+ser.portstr
else:
	print "Error opening serial port"
	sys.exit()
ser.flushInput()
ser.flushOutput()
while(True):
	input = raw_input("Please make a selection: (\033[1mR\033[0m)un Setup, or (\033[1mQ\033[0m)uit\n")
	if(input == "R"):
		print "Running setup program. Please follow prompts. If an error occurs, contact Nathan"
		writeObject(ser, "o1901.2", "5") #5 is the command to set up the motor or recover from errors
		response = readObject(ser, "o1901.3")
                print "response,", response
		if(float(response)==1): #1 is command acknowledged
			print "Motor accepted setup command."
		else:
		        print response
                        print "Error! Motor will not accept setup command. Exiting."
			sys.exit()
		while(True):
			print "Are both the flapper and the trapdoor ready to be adjusted?"
			print " - For the flapper, the plate must be free of obstructions and the turn-buckle properly adjusted to allow full travel in 360 degrees. Additionally, the switches must be wired properly and working. Incorrect switch wiring may lead to the trapdoor crashing into the flapper and serious damage to the mechanisms.\n"
			print " - For the trapdoor, the worm gear must be fully engaged and the shaft must be at the negative limit switch. The negative, positive, and home limit switches must be wired correctly or the motor may move the shaft out of gear. The gear must be clear of obstructions and ready to move. The motor needs to be attached to the worm gear."
			input = raw_input("Please indicate if the flapper and trapdoor are ready: (Y/N)\n")
			if(input == "Y" or input== "y"):
				print "Sending motor command to begin alignment procedures"
				writeObject(ser, "o1901.7", "1")
				response = readObject(ser, "o1901.8")
				if(float(response)==1): #1 is setup flapper
					print "Motor reports flapper is not in a safe position for trapdoor homing.\nPlease manually adjust flapper position by rotating flapper stepper motor disk until the flapper is in the up position. The stepper motor will lock when the proper position is reached. If resistance is encountered but the stepper motor does not lock the disk (motion backwards is still easy), try turning the disk in the opposite direction. If the position does not lock automatically, please abort alignment procedure and contact Nathan."
					while(True):
						print "Was the flapper positioning successful? Verify that the flapper is in the up position. If it is in the up position, the teardrop shaped link attached to the red turn-buckle should be pointing as far up as possible."
						input = raw_input("Is the flapper in the up position? ((\033[1mY\033[0m)es or (\033[1mN\033[0m)o The motor locked but in the wrong position or the motor did not lock\n")
						if(input == "N"):
							print "There is an error with the switches, and the up position switch does not work properly. Please contact Nathan to reslove this issue."
							writeObject(ser, "o1901.7", "-1")
							sys.exit()
						elif(input == "Y"):
							writeObject(ser, "o1901.7", "2") #2 is setup shaft
							break
						else:
							print "Unknown command. Please enter a known command."
				elif(float(response)==2):
					print "Motor reports the flapper is in the up position. Please verify that is correct by looking at the stepper motor. If it is in the up position, the teardrop shaped link attached to the red turn-buckle should be pointing as far up as possible."
					while(True):
						input = raw_input("Is the flapper in the up position? ((\033[1mY\033[0m)es or (\033[1mN\033[0m)o\n")
						if(input == "N"):
							print "There is an error with the switches, and the up position switch does not work properly. Please contact Nathan to reslove this issue."
							writeObject(ser, "o1901.7", "-1")
							sys.exit()
						elif(input == "Y"):
							writeObject(ser, "o1901.7", "2") #2 is setup shaft
							break
						else:
							print "Unknown command. Please enter a known command."
				else:
					print "Unknown motor response. Please contact Nathan. Quitting."
					print response
					sys.exit()
				
				print "Motor is currently homing. This program will print a message upon completion and quit."
				while(True):
					response = readObject(ser, "o1901.8")
					print("RESP: "+response)
					if(float(response) == 3):
						print "Motor homed properly. Exiting."
						break
					elif(float(response) == 4):
						print "Motor currently homing. Waiting for motor to finish"
					elif(float(response) == 5):
						print "An error has occured. Please consult error manual"
						print "Error: "+readObject(ser, "o1901.9")
						sys.exit()
					else:
						print "The motor has responded in an unextpected way."
					
					response = readObject(ser, "o1901.1")
					if(float(response) == 2):
						print "An error has occured. Error: "+readObject(ser,"o1901.4")+"Exiting."
						sys.exit()

					time.sleep(.5)
				break
				
			elif(input == "N"):
				print "Please  set up motors manually so that they are ready to be aligned. Quitting program."
				sys.exit()
			else:
				print "Unknown command. Please enter a known command."
	elif(input == "Q"):
		break

	else:
		print "Unknown command. Please enter a known command."

print "Thank you for using the setup program."
