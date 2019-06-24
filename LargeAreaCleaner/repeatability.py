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
sock.sendto(eSCL_Header+'AC70\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'DE70\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'VE70\r', ("10.10.10.10", 7775))
time.sleep(0.1)

for x in range(10):
	sock.sendto(eSCL_Header+'FL10387500\r', ("10.10.10.10", 7775))
	time.sleep(15)
	sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
	data, addr = sock.recvfrom(1024)
	print "Up Pos: "+data
	sock.sendto(eSCL_Header+'FL-10387500\r', ("10.10.10.10", 7775))
	time.sleep(15)
	sock.sendto(eSCL_Header+'IE\r', ("10.10.10.10", 7775))
	data, addr = sock.recvfrom(1024)
	print "Down Pos: "+data


#while True:
#data = sock.recv(1024)
#print data
