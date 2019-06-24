#!/usr/bin/python

import sys
import serial
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
sys.path.insert(1,'TrapdoorMotorPythonPrograms/')
from TrapDoorControl import TrapDoorControl
from subprocess import call

TESTMODE = True # if True, commands are not actually sent to TrapDoorControl

ftdi_serial_id = 'A603J6ZY' # unique serial number for the USB-serial converter
call(["../utils/find_usbserial_name.pl",ftdi_serial_id])
com_port = '../ports/ttyftdi' + ftdi_serial_id

trapdoor = None
if not TESTMODE:
    trapdoor = TrapDoorControl(com_port)

logfile = open('trapdoor.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': Trap door server started\n')

if TESTMODE:
    testfile = open('test_trapdoor.txt', 'a')

try:
    # Setup the ZeroMQ objects
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind('ipc:///tmp/zeromq_trapdoor')

    state_str = 'DONT KNOW'

    # Listen for commands over the wire.
    print "Listening..."
    while True:
        command = socket.recv()
        if command == 'SET FILL':
            logstring = datetime.now().isoformat(' ') + ': SETTING TO FILL'
            state_str = 'FILL'
            reply_str = 'SETTING TO ' + state_str
            socket.send(reply_str)
            print logstring
            logfile.write(logstring + '\n')
            if TESTMODE:
                testfile.write('Set to fill\n')
            else:
                trapdoor.fill()
        elif command == 'SET HOLD':
            logstring = datetime.now().isoformat(' ') + ': SETTING TO HOLD'
            state_str = 'HOLD'
            reply_str = 'SETTING TO ' + state_str
            socket.send(reply_str)
            print logstring
            logfile.write(logstring + '\n')
            if TESTMODE:
                testfile.write('Set to hold\n')
            else:
                trapdoor.hold()
        elif command == 'SET DUMP':
            logstring = datetime.now().isoformat(' ') + ': SETTING TO DUMP'
            state_str = 'DUMP'
            reply_str = 'SETTING TO ' + state_str
            socket.send(reply_str)
            print logstring
            logfile.write(logstring + '\n')
            if TESTMODE:
                testfile.write('Set to dump\n')
            else:
                trapdoor.dump()
        elif command == 'SET UP':
            logstring = datetime.now().isoformat(' ') + ': SETTING TO UP'
            state_str = 'UP'
            reply_str = 'SETTING TO ' + state_str
            socket.send(reply_str)
            print logstring
            logfile.write(logstring + '\n')
            if TESTMODE:
                testfile.write('Set to up\n');
            else:
                trapdoor.up()
        elif command == 'GET STATE':
            socket.send('STATE IS ' + state_str)
        else:
            logstring = datetime.now().isoformat(' ') + ': Command not recognized [' + command + ']'
            print logstring
            logfile.write(logstring + '\n')
            socket.send('COMMAND NOT RECOGNIZED')


finally:
    if TESTMODE:
        testfile.close()
    logfile.close()


