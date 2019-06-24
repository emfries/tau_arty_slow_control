#!/usr/bin/python

import sys
import zmq
from time import sleep

try:
    position_str = sys.argv[1]
except:
    print 'Usage: ./set_dagger_position.py [position in steps]'
    sys.exit(2)


context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_dagger')

socket.send('RESET VEL ' + position_str)
message = socket.recv()
print 'Received reply ' + message

sleep(0.1)

socket.send('SET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

