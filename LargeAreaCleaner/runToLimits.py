#!/usr/bin/python

import socket
import time

#UDP_IP = "10.10.10.10"
#UDP_PORT = 7775
#MESSAGE = "MN\r"

eSCL_Header = "\000\007" #Needs to be first part so drive knows it's an eSCL command

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("10.10.10.11", 7774))
#sock.sendto(eSCL_Header+"MN\r", ("10.10.10.10", 7775))


sock.sendto(eSCL_Header+'DL2\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'AC5\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'DE5\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'VE5\r', ("10.10.10.10", 7775))
time.sleep(0.1)

sock.sendto(eSCL_Header+'FL20000000\r', ("10.10.10.10", 7775))
sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
data, addr = sock.recvfrom(1024)
print "lowering: "+data
prevData = data

while True:
	time.sleep(1.0)
	sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
	data, addr = sock.recvfrom(1024)
	print "lowering: "+data
	if(data == prevData):
		break
	prevData = data

print "lowest: "+data

sock.sendto(eSCL_Header+'FL-20000000\r', ("10.10.10.10", 7775))
sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
data, addr = sock.recvfrom(1024)
print "raising: "+data
prevData = data

while True:
	time.sleep(1.0)
	sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
	data, addr = sock.recvfrom(1024)
	print "raising: "+data
	if(data == prevData):
		break
	prevData = data

print "highest: "+data
