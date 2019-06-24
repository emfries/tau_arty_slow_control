import sys
import zmq
import time

try:
    position_str = sys.argv[1]
except:
    print 'Usage: ./set_dagger_position.py [position in steps]'
    sys.exit(2)


print 'connecting'
context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect('ipc:///tmp/zeromq_dagger')#_pulseblaster')
time.sleep(10)

print 'connected'

socket.send('SET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

time.sleep(10)

socket.send('GET Z ' + position_str)
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

#for i in range(10):
#    pos = i*10000
#    socket.send('SET Z ' + str(pos))
#    message = socket.recv()
#    print 'Received reply ' + message
#    #i = i + 1
#    time.sleep(10)

socket.send('SET Z ' + '490000')
message = socket.recv()
print 'Received reply ' + message

time.sleep(30)

socket.send('SET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

socket.send('SET Z ' + '0')
message = socket.recv()
print 'Received reply ' + message

socket.send('GET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

time.sleep(30)

socket.send('GET Z ' + position_str)
message = socket.recv()
print 'Received reply ' + message

