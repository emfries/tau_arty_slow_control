#!/usr/bin/python

#See Host Command Reference for an explanation of commands.

import sys
import socket
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from time import sleep
from subprocess import call

# Change these if necessary
gCleanAddr = ("10.10.10.21", 7776)

z_max = 10387500 # Steps
z_min = 100000 # Steps

TESTMODE = False # if True, commands are written to test_giantCleaner.txt instead of the serial port.

accel_str = '10'  # rev/s/s
decel_str = '10'  # rev/s/s
vel_str = '10'  # rev/s

#accel_str = '10'  # rev/s/s
#decel_str = '10'  # rev/s/s
#vel_str = '10'  # rev/s

termchar = '\r'
eSCL_Header = "\000\007" #Needs to be first part so drive knows it's an eSCL command


logfile = open('giantCleaner.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': Giant cleaner server started. \n')

if TESTMODE:
	sock = open('test_giantCleaner.txt', 'a')
	termchar = '\n'
else:
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.bind(("10.10.10.21", 7776))

def set_motor_params():
	if TESTMODE:
		vel_settings = 'AC' + accel_str + ', DE' + decel_str + ', VE' + vel_str
		logfile.write(datetime.now().isoformat(' ') + ': ' + vel_settings + '\n')
		return vel_settings
	# Set the motor acceleration, deceleration, and velocity
	#sock.sendto(eSCL_Header+'DL2\r', gCleanAddr)
	sock.sendto(eSCL_Header + 'AC' + accel_str + termchar, gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header + 'DE' + decel_str + termchar, gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header + 'VE' + vel_str + termchar, gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header + 'DL2' + termchar, gCleanAddr) #Set limit switch behavior
	sock.sendto(eSCL_Header + 'MO1' + termchar, gCleanAddr)
	#sock.sendto(eSCL_Header + eSCL_Header + 'MO1' + termchar, gCleanAddr) # Enable motion bit Y2
	sock.sendto(eSCL_Header + 'SI3' + termchar, gCleanAddr) #Enable input 3 for waiting on input
	sleep(0.1)
	vel_settings = 'AC' + accel_str + ', DE' + decel_str + ', VE' + vel_str
	logfile.write(datetime.now().isoformat(' ') + ': ' + vel_settings + '\n')
	return vel_settings

def find_limits():
	if TESTMODE:
		print "Skipping find_limits for TESTMODE"
		return 0

	sock.sendto(eSCL_Header+'IFD\r', gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header+'DL2\r', gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header+'AC5\r', gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header+'DE5\r', gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header+'VE5\r', gCleanAddr)
	sleep(0.1)
	sock.sendto(eSCL_Header + 'MO1' + termchar, gCleanAddr)
	sleep(0.1)

	sock.sendto(eSCL_Header+'IS\r', gCleanAddr) #Get status of digital inputs
	inputs, addr = sock.recvfrom(1024)

	#\000\007IS=xxxxxxxx
	if inputs[2:5] == 'IS=':
		at_lim = inputs[12]
	else: 
		print 'Improper inputs response from motor controller. Exiting.'
		sys.exit()

	if(at_lim == '1'): #Limits are active open
		print 'Already at Limit; Moving up!'
		sock.sendto(eSCL_Header+'FL-1000000\r', gCleanAddr) #Back off to run back down to lim
		sleep(10.0)

	sock.sendto(eSCL_Header+'FL100000000\r', gCleanAddr)
	sock.sendto(eSCL_Header+'IE\r', gCleanAddr)
	data, addr = sock.recvfrom(1024)
	#print "lowering: "+data
	prevData = data

	while True:
		sleep(1.0)
		sock.sendto(eSCL_Header+'IE\r', gCleanAddr)
		data, addr = sock.recvfrom(1024)
		print "lowering: "+data
		if(data == prevData):
			break
		prevData = data

	print "Found low limit!"

	sock.sendto(eSCL_Header+'EP0\r', gCleanAddr)
	sock.sendto(eSCL_Header+'SP0\r', gCleanAddr)

	print "Setting Stall Prevention mode!"

	sock.sendto(eSCL_Header+'CC5\r', gCleanAddr)
	sock.sendto(eSCL_Header+'CI4.5\r', gCleanAddr)
	sleep(1.0)
	sock.sendto(eSCL_Header+'EF6\r', gCleanAddr)
	sleep(1.0)
	sock.sendto(eSCL_Header+'CC3\r', gCleanAddr)

	set_motor_params()

try:
	if TESTMODE:
		status = 'SC=0001\n'
	else:
		sock.sendto(eSCL_Header + 'SC' + termchar, gCleanAddr)  # Request status word from the motor controller
		status, addr = sock.recvfrom(1024)

	print 'Status reply from motor controller: ' + status
	if status[2:5] == 'SC=': #Responses contain the eSCL_Header.
		logfile.write(datetime.now().isoformat(' ') + ': Motor controller status: ' + status + '\n')
		print 'Motor controller status: ' + status
	else:
		print 'No response from motor controller. Exiting.'
		sys.exit()

	find_limits()
	set_motor_params()

	# Setup the ZeroMQ objects
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind('ipc:///tmp/zeromq_gcPulseBlast')

	# Listen for commands over the wire.
	print 'Listening...'
	while True:
		command = socket.recv()
		if command[0:5] == 'SET Z':
			#sock.sendto(eSCL_Header + 'IE' + termchar, gCleanAddr)
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
			if TESTMODE:
				sock.write(motor_command + termchar)
			else:
				sock.sendto(eSCL_Header + motor_command + termchar, gCleanAddr)
			#encoder_pos = ser.readline()
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
			if TESTMODE:
				sock.write('WI3L' + termchar)
				sock.write(motor_command + termchar)
			else:
				sock.sendto(eSCL_Header + 'WI3L' + termchar, gCleanAddr)
				sleep(0.1)
				sock.sendto(eSCL_Header + motor_command + termchar, gCleanAddr)
			#encoder_pos = ser.readline()
			logstring = datetime.now().isoformat(' ') + ': ' + command + ', (WI3L; ' + motor_command + ')'
			print logstring
			logfile.write(logstring + '\n')
			reply_str = 'QUEUED Z ' + str(z_request)
			socket.send(reply_str)
		elif command[0:5] == 'GET Z':
			sock.sendto(eSCL_Header + 'IP' + termchar, gCleanAddr)
			resp, addr = sock.recvfrom(1024)
			reply_str = 'Z ' + resp.split('=')[1]
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMIN':
			reply_str = 'ZMIN ' + str(z_min)
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMAX':
			reply_str = 'ZMAX ' + str(z_max)
			socket.send(reply_str)
		#print 'Setting AC='+accel_str + ', DE='+decel_str + ', VE=' + vel_str
		elif command[0:9] == 'RESET VEL':
			# Set the motor acceleration, deceleration, and velocity
			vel_settings = set_motor_params()
			socket.send(vel_settings)
		elif command[0:4] == 'ABRT':
			if TESTMODE:
				sock.write('SK')
			else:
				sock.sendto(eSCL_Header + 'SK' + termchar, gCleanAddr)
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
	sock.close()
	logfile.close()

