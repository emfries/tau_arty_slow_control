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
sock.sendto(eSCL_Header+'AC15\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'DE15\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'VE10\r', ("10.10.10.10", 7775))
time.sleep(0.1)
sock.sendto(eSCL_Header+'FL100000000\r', ("10.10.10.10", 7775))


#while True:
#data = sock.recv(1024)
#print data
