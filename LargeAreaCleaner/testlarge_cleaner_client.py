import sys
import zmq
import time

try:
    position_str = sys.argv[1]
except:
    print 'Usage: ./set_dagger_position.py [position in steps]'
    sys.exit(2)


context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_dagger')

socket.send('SET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

socket.send('GET ZMIN')
message = socket.recv()
print 'Received reply ' + message

socket.send('GET ZMAX')
message = socket.recv()
print 'Received reply ' + message

socket.send('GOBBLEDYGOOK')
message = socket.recv()
print 'Received reply ' + message

i = 100
while True:
    socket.send('SET Z ' + str(i))
    message = socket.recv()
    print 'Received reply ' + message
    i = i + 1
    time.sleep(2)

