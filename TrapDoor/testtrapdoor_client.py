import sys
import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_trapdoor')

socket.send('SET HOLD')
message = socket.recv()
print 'Received reply ' + message

socket.send('SET FILL')
message = socket.recv()
print 'Received reply ' + message

socket.send('SET DUMP')
message = socket.recv()
print 'Received reply ' + message

socket.send('SET UP')
message = socket.recv()
print 'Received reply ' + message

socket.send('GOBBLEDYGOOK')
message = socket.recv()
print 'Received reply ' + message

socket.send('GET STATE')
message = socket.recv()
print 'Received reply ' + message


