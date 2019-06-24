#!/usr/bin/python

from TrapDoorControl import *
import serial, time, sys

trapdoorControl = TrapDoorControl('../../ports/ttyftdiA603J6ZY')

while(True):
    print "Told motor to Fill."
    trapdoorControl.fill()
    time.sleep(5)
    
    print "Told motor to Up."
    trapdoorControl.up()
    time.sleep(5)
    
    print "Told motor to Dump."
    trapdoorControl.dump()
    time.sleep(5)

print "Thank you for using your mom's program!"
