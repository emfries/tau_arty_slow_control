#!/usr/bin/python

from TrapDoorControl import *
import serial, time, sys

sleeptime=300
trapdoorControl = TrapDoorControl('../../ports/ttyftdiA603J6ZY')
n_iterations = 10
for i in range(n_iterations):
        print "--->iteration ",i
    #print "Told motor to Fill."
        trapdoorControl.fill()
        time.sleep(sleeptime)
	switchState = trapdoorControl.getSwitchState()
	bytes = ' '.join(format(ord(x), 'b') for x in switchState)
	print "Fill_Run_Begin: "+bytes
	time.sleep(1)
        
        #print "Told motor to Up."
        trapdoorControl.up()
        time.sleep(sleeptime)
	switchState = trapdoorControl.getSwitchState()
	bytes = ' '.join(format(ord(x), 'b') for x in switchState)
	print "Up: "+bytes
	time.sleep(1)
        
        #print "Told motor to Dump."
        trapdoorControl.dump()
        time.sleep(sleeptime)
	switchState = trapdoorControl.getSwitchState()
	bytes = ' '.join(format(ord(x), 'b') for x in switchState)
	print "Dump: "+bytes
	time.sleep(1)
	
	trapdoorControl.fill()
        time.sleep(30)
	switchState = trapdoorControl.getSwitchState()
	bytes = ' '.join(format(ord(x), 'b') for x in switchState)
	print "Fill_Run_End: "+bytes
	time.sleep(1)

trapdoorControl.up()
