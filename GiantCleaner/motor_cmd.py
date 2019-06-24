#!/usr/bin/python

import sys
import socket
import string
from time import sleep

# Change these if necessary

motor_ip = "10.10.10.14"

TESTMODE = False # if True, commands are written to test_giantCleaner.txt instead of the serial port.

accel_str = '10'  # rev/s/s
decel_str = '10'  # rev/s/s
vel_str = '1'  # rev/s
CC_str = '2.0'

termchar = '\r'
eSCL_Header = "\000\007" #Needs to be first part so drive knows it's an eSCL command

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("10.10.10.11", 7774))
sock.settimeout(0.5)

sock.sendto(eSCL_Header + 'AC' + accel_str + termchar, (motor_ip, 7775))
sleep(0.1)
sock.sendto(eSCL_Header + 'DE' + decel_str + termchar, (motor_ip, 7775))
sleep(0.1)
sock.sendto(eSCL_Header + 'VE' + vel_str + termchar, (motor_ip, 7775))
sleep(0.1)

while(True):
	try:
		command = raw_input("[TXM34] > ")
	except EOFError, KeyboardInterrupt:
		print "Thanks for Using the program!"
		break
	sock.sendto(eSCL_Header + command + termchar, (motor_ip, 7775))
	try:
		data = sock.recv(1024)
	except socket.timeout:
		data = "No Response"
	print "[TXM34] < "+data
	
sock.close()
