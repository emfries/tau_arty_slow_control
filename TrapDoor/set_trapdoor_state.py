#!/usr/bin/python

import sys
import zmq

try:
    state_str = sys.argv[1]
except:
    print 'Usage: ./set_trapdoor_state.py [FILL/HOLD/DUMP/UP]'
    sys.exit(2)


context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_trapdoor')

socket.send('SET ' + state_str)
message = socket.recv()
print 'Received reply ' + message

