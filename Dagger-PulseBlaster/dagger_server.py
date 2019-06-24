#!/usr/bin/python

#See Host Command Reference for an explanation of commands.

import sys
import serial
import zmq	# ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from time import sleep
from subprocess import call

# Change these if necessary
z_max = 490000   # Steps (slightly more than 1 micron; don't be fooled)
z_min = 10000	# Steps (slightly more than 1 micron; don't be fooled)
# z_current = z_max  # Assuming we start this server when the dagger is up.

TESTMODE = False # if True, commands are written to test_dagger.txt instead of the serial port.

accel_str = '6'  # rev/s/s
decel_str = '6'  # rev/s/s
vel_str = '1.6'  # rev/s

ftdi_serial_id = 'A7032WB0'  # unique serial number for USB-serial converter
call(["../utils/find_usbserial_name.pl",ftdi_serial_id])
com_port = '../ports/ttyftdi' + ftdi_serial_id

print 'Using port ' + com_port

termchar = '\r'


logfile = open('dagger.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': Dagger server started. ')# Assume z=' + str(z_current) + ' steps.\n')

if TESTMODE:
	ser = open('test_dagger.txt', 'a')
	termchar = '\n'
else:
	ser = serial.Serial(com_port, baudrate=9600, bytesize=8, parity='N', stopbits=1, timeout=1)


def set_motor_params():
	# Set the motor acceleration, deceleration, and velocity
	ser.write('AC' + accel_str + termchar)
	sleep(0.1)
	ser.write('DE' + decel_str + termchar)
	sleep(0.1)
	ser.write('VE' + vel_str + termchar)
	sleep(0.1)
	ser.write('MO1' + termchar) # Enable motion bit Y2
	sleep(0.1)
	ser.write('JD' + termchar)
	vel_settings = 'AC' + accel_str + ', DE' + decel_str + ', VE' + vel_str
	logfile.write(datetime.now().isoformat(' ') + ': ' + vel_settings + '\n')
	return vel_settings

def startup_setup():
	ser.write('IFD' + termchar)
	sleep(0.1)
	ser.write('CC' + termchar) #Get current
	response = ser.read(128)
	current = response.split("=")[1]
	print "Allowing dagger to fall!"
	ser.write('CC0' + termchar)   #Set current to 0 (allows dagger to fall to hard stop gently)
	ser.write('IE' + termchar)   #Get encoder position
	lastPos = ser.read(128)
	while True:
		print lastPos
		sleep(1.0)  #poll every 1s
		ser.write('IE' + termchar) #Get next encoder position
		curPos = ser.read(128)
		if curPos == lastPos: #If it's the same twice the dagger is on hard stop
			break
		lastPos = curPos
	ser.write('EP0' + termchar) #Zero Encoder
	ser.write('SP0' + termchar) #Zero current position
	print "Setting Stall Prevention mode!"
	print "Current: " + current
	ser.write('CC10' + termchar) #Max out current before enabling stall prevention
	sleep(0.1)
	ser.write('CI9' + termchar)
	sleep(1.0)
	ser.write('EF6' + termchar)
	sleep(1.0)
	ser.write('CC' + current)
	
	set_motor_params()

try:
	ser.write('SC' + termchar)  # Request status word from the motor controller
	if TESTMODE:
		status = 'SC=0001\n'
	else:
		status = ser.read(20)

	print 'Status reply from motor controller: ' + status
	if status[0:3] == 'SC=':
		logfile.write(datetime.now().isoformat(' ') + ': Motor controller status: ' + status + '\n')
		print 'Motor controller status: ' + status
	else:
		print 'No response from motor controller. Exiting.'
		sys.exit()

	startup_setup()

	# Setup the ZeroMQ objects
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind('ipc:///tmp/zeromq_dagger_pulseblaster')

	# Listen for commands over the wire.
	print 'Listening...'
	while True:
		command = socket.recv()
		if command[0:5] == 'SET Z':
			#ser.write('IE' + termchar)
			z_request = string.atoi(command[5:])
			if z_request < z_min:
				z_request = z_min
				logstring = datetime.now().isoformat(' ') + ': Requested position < z_min (' + str(z_min) + '), setting to z_min.'
				print logstring
				logfile.write(logstring + '\n')
			if z_request > z_max:
				z_request = z_max
				logstring = datetime.now().isoformat(' ') + ': Requested position > z_max (' + str(z_max) + '), setting to z_max.'
				print logstring
				logfile.write(logstring + '\n')
			# Note: upward movement is negative!
			motor_command = 'FP' + str(-1*z_request)
			ser.write(motor_command + termchar)
			logstring = datetime.now().isoformat(' ') + ': ' + command + ', (' + motor_command + ')'
			print logstring
			logfile.write(logstring + '\n')
			reply_str = 'Z ' + str(z_request)
			socket.send(reply_str)
		elif command[0:7] == 'QUEUE Z':
			z_request = string.atoi(command[7:])
			if z_request < z_min:
				z_request = z_min
				logstring = datetime.now().isoformat(' ') + ': Requested position < z_min (' + str(z_min) + '), setting to z_min.'
				print logstring
				logfile.write(logstring + '\n')
			if z_request > z_max:
				z_request = z_max
				logstring = datetime.now().isoformat(' ') + ': Requested position > z_max (' + str(z_max) + '), setting to z_max.'
				print logstring
				logfile.write(logstring + '\n')
			# Note: upward movement is negative!
			motor_command = 'FP' + str(-1*z_request)
			ser.write('WI1L' + termchar)
			sleep(0.1)
			ser.write(motor_command + termchar)
			logstring = datetime.now().isoformat(' ') + ': ' + command + ', (WI1L; ' + motor_command + ')'
			print logstring
			logfile.write(logstring + '\n')
			reply_str = 'QUEUED Z ' + str(z_request)
			socket.send(reply_str)
		elif command[0:5] == 'GET Z':
			ser.write('IP' + termchar)
			resp = ser.read(128)
			reply_str = 'Z ' + resp.split('=')[1]
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMIN':
			reply_str = 'ZMIN ' + str(z_min)
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMAX':
			reply_str = 'ZMAX ' + str(z_max)
			socket.send(reply_str)
			print 'Setting AC='+accel_str + ', DE='+decel_str + ', VE=' + vel_str
		elif command[0:9] == 'RESET VEL':
			# Set the motor acceleration, deceleration, and velocity
			vel_settings = set_motor_params()
			socket.send(vel_settings)
		elif command[0:4] == 'ABRT':
			ser.write('SKD' + termchar)
			logstring = datetime.now().isoformat(' ') + ': Aborted Queue and Stopped Movement'
			logfile.write(logstring + '\n')
			socket.send('ABORTED')
		else:
			logstring = datetime.now().isoformat(' ') + ': Command not recognized [' + command + ']'
			print logstring
			logfile.write(logstring + '\n')
			socket.send('COMMAND NOT RECOGNIZED')


finally:
	logfile.write(datetime.now().isoformat(' ') + ': Server stopped.\n')
	ser.close()
	logfile.close()

