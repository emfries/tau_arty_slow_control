#!/usr/bin/python

import sys
import zmq
from time import sleep



context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_dagger')

socket.send('GET Z')
message = socket.recv()
print 'Received reply ' + message

