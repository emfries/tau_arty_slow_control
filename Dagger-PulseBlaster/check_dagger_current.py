#!/usr/bin/python

import sys
import serial
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from time import sleep
from subprocess import call

# Change these if necessary
z_max = 490000   # Steps (slightly more than 1 micron; don't be fooled)
z_min = 10000    # Steps (slightly more than 1 micron; don't be fooled)
# z_current = z_max  # Assuming we start this server when the dagger is up.

TESTMODE = False # if True, commands are written to test_dagger.txt instead of the serial port.

ftdi_serial_id = 'A7032WB0'  # unique serial number for USB-serial converter
call(["../utils/find_usbserial_name.pl",ftdi_serial_id])
com_port = '../ports/ttyftdi' + ftdi_serial_id

ser = serial.Serial(com_port, baudrate=9600, bytesize=8, parity='N', stopbits=1, timeout=1)

print 'Using port ' + com_port

termchar = '\r'

#ser.write('CC' + termchar) #Get current
#response = ser.read(128)
#print "Response: " + response
#current = response.split("=")[1]
#print "Current: " + current
ser.write('IFD' + termchar)
ser.write('IP' + termchar)
resp = ser.read(128)
print "Response: " + resp
#print "POS: " + str(int(resp.split('=')[1], 16))
ser.close()


