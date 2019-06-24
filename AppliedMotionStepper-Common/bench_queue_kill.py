#!/usr/bin/python

import sys
import socket
import string
from datetime import datetime
from time import sleep
from subprocess import call

# Change these if necessary
z_max = 1038750 # Steps
z_min = 10000 # Steps
z_current = z_max  # Assuming we start this server when the giantCleaner is up.



TESTMODE = False # if True, commands are written to test_giantCleaner.txt instead of the serial port.

accel_str = '5'  # rev/s/s
decel_str = '5'  # rev/s/s
vel_str = '5'  # rev/s

termchar = '\r'
eSCL_Header = "\000\007" #Needs to be first part so drive knows it's an eSCL command


logfile = open('giantCleaner.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': Giant cleaner server started.  Assume z=' + str(z_current) + ' steps.\n')

if TESTMODE:
    sock = open('test_giantCleaner.txt', 'a')
    termchar = '\n'
else:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("10.10.10.11", 7774))

try:
    if TESTMODE:
        status = 'SC=0001\n'
    else:
        sock.sendto(eSCL_Header + 'SC' + termchar, ("10.10.10.10", 7775))  # Request status word from the motor controller
        status, addr = sock.recvfrom(1024)
        sock.sendto(eSCL_Header + 'BS' + termchar, ("10.10.10.10", 7775))  # Request status word from the motor controller
        buf_status, addr = sock.recvfrom(1024)

    print 'Status reply from motor controller: ' + status
    print 'Buffer Status reply from motor controller: ' + buf_status
    if status[2:5] == 'SC=': #Responses contain the eSCL_Header.
        logfile.write(datetime.now().isoformat(' ') + ': Motor controller status: ' + status + '\n')
        print 'Motor controller status: ' + status
    else:
        print 'No response from motor controller. Exiting.'
        sys.exit()

    #z_current = find_limits()
    z_current = 50000
    
    if TESTMODE:
        sock.write(motor_command + termchar)
    else:
		sock.sendto(eSCL_Header + 'SK' + termchar, ("10.10.10.10", 7775))
		sleep(0.1)

    # Setup the ZeroMQ objects
    #context = zmq.Context()
    #socket = context.socket(zmq.REP)
    #socket.bind('ipc:///tmp/zeromq_giantCleaner')

    # Listen for commands over the wire.
    #~ print 'Listening...'
    #~ while True:
        #~ command = socket.recv()
        #~ if command[0:5] == 'SET Z':
            #~ #sock.sendto(eSCL_Header + 'IE' + termchar, ("10.10.10.10", 7775))
            #~ z_request = string.atoi(command[5:])
            #~ if z_request < z_min:
                #~ z_request = z_min
                #~ logstring = datetime.now().isoformat(' ') + ': Requested position < z_min (' + str(z_min) + '), setting to z_min.'
                #~ print logstring
                #~ logfile.write(logstring + '\n')
            #~ if z_request > z_max:
                #~ z_request = z_max
                #~ logstring = datetime.now().isoformat(' ') + ': Requested position > z_max (' + str(z_max) + '), setting to z_max.'
                #~ print logstring
                #~ logfile.write(logstring + '\n')
            #~ displacement = z_request - z_current
            #~ if displacement != 0:
                #~ # Note: upward movement is negative!
                #~ motor_command = 'FL' + str(-1*displacement)
                #~ if TESTMODE:
                    #~ sock.write(motor_command + termchar)
                #~ else:
                    #~ sock.sendto(eSCL_Header + motor_command + termchar, ("10.10.10.10", 7775))
            #~ else:
                #~ motor_command = '(already in this position)'
            #~ #encoder_pos = ser.readline()
            #~ z_current = z_request
            #~ logstring = datetime.now().isoformat(' ') + ': ' + command + ', (' + motor_command + ')'
            #~ print logstring
            #~ logfile.write(logstring + '\n')
            #~ reply_str = 'Z ' + str(z_current)
            #~ socket.send(reply_str)
        #~ elif command[0:8] == 'GET Z':
            #~ reply_str = 'Z ' + str(z_current)
            #~ socket.send(reply_str)
        #~ elif command[0:8] == 'GET ZMIN':
            #~ reply_str = 'ZMIN ' + str(z_min)
            #~ socket.send(reply_str)
        #~ elif command[0:8] == 'GET ZMAX':
            #~ reply_str = 'ZMAX ' + str(z_max)
            #~ socket.send(reply_str)
            #~ print 'Setting AC='+accel_str + ', DE='+decel_str + ', VE=' + vel_str
        #~ elif command[0:9] == 'RESET VEL':
            #~ # Set the motor acceleration, deceleration, and velocity
            #~ vel_settings = set_motor_params()
            #~ socket.send(vel_settings)
        #~ else:
            #~ logstring = datetime.now().isoformat(' ') + ': Command not recognized [' + command + ']'
            #~ print logstring
            #~ logfile.write(logstring + '\n')
            #~ socket.send('COMMAND NOT RECOGNIZED')


finally:
    logfile.write(datetime.now().isoformat(' ') + ': Server stopped.\n')
    sock.close()
    logfile.close()

