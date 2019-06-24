#!/usr/bin/python

import sys
import serial
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from time import sleep
from subprocess import call

TESTMODE = False # if True, commands are written to test_dagger.txt instead of the serial port.

ftdi_serial_id = 'A7032WB0'  # unique serial number for USB-serial converter
call(["../../utils/find_usbserial_name.pl",ftdi_serial_id])
com_port = '../../ports/ttyftdi' + ftdi_serial_id

print 'Using port ' + com_port

termchar = '\r'

ser = serial.Serial(com_port, baudrate=9600, bytesize=8, parity='N', stopbits=1, timeout=1)

ser.write('SC' + termchar)  # Request status word from the motor controller
status = ser.read(20)
print 'Status reply from motor controller: ' + status
if status[0:3] == 'SC=':
	print 'Motor controller status: ' + status
else:
	print 'No response from motor controller. Exiting.'
	sys.exit()

ser.write('CC' + termchar)  # Request status word from the motor controller
currentCC = ser.read(20)
print 'currnet: ' + currentCC

ser.write('CC10' + termchar)  # Request status word from the motor controller

ser.write('CC' + termchar)  # Request status word from the motor controller
currentCC = ser.read(20)
print 'currnet: ' + currentCC

ser.close()

