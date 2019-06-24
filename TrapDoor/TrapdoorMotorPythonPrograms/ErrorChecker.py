#!/usr/bin/python

from TrapDoorControl import *
import serial, time, sys

trapdoorControl = TrapDoorControl('../../ports/ttyftdiA603J6ZY')

error = trapdoorControl.readObject("o550.1")
print error
error = trapdoorControl.readObject("o550.2")
print error
error = trapdoorControl.readObject("o550.3")
print error
error = trapdoorControl.readObject("o550.4")
print error
error = trapdoorControl.readObject("o550.5")
print error
error = trapdoorControl.readObject("o550.6")
print error
error = trapdoorControl.readObject("o550.7")
print error

print "Thank you for using the Error Checker Program! May your errors forever be obscure."
