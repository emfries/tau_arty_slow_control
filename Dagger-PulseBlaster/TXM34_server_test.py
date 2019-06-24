import sys
import socket
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from time import sleep
from subprocess import call

# Change these if necessary
z_max = 490000   # Steps (slightly more than 1 micron; don't be fooled)
z_min = 10000	# Steps (slightly more than 1 micron; don't be fooled)
# z_current = z_max  # Assuming we start this server when the dagger is up.

TESTMODE = False # if True, commands are written to test_dagger.txt instead of the serial port.

accel_str = '6'  # rev/s/s
decel_str = '6'  # rev/s/s
vel_str   = '1.6'  # rev/s
eg_str    = '25400' #steps/rev --electronic gearing
cc_str    = '2.0'  #amps drive current
ci_str    = '1.0'  #amps idle current

termchar = '\r'
eSCL_Header = "\000\007" #Needs to be first part so drive knows it's an eSCL command

motor_address= ("10.10.10.14", 7775)#used in 2017-18 production
#motor_address= ("10.10.10.21", 7775)#for testing

logfile = open('dagger.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': Dagger server started. \n')# Assume z=' + str(z_current) + ' steps.\n')

if TESTMODE:
	ser = open('test_dagger.txt', 'a')
	termchar = '\n'
else:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("10.10.10.21", 7774))


def set_motor_params():
	# Set the motor acceleration, deceleration, and velocity
	sock.sendto(eSCL_Header + 'AC' + accel_str + termchar, motor_address)
	sleep(0.1)
        resp,addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'DE' + decel_str + termchar, motor_address)
	sleep(0.1)
        resp,addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'VE' + vel_str + termchar, motor_address)
	sleep(0.1)
        resp,addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'EG' + eg_str + termchar, motor_address)
	sleep(0.1)
        resp,addr = sock.recvfrom(1024)
	#sock.sendto(eSCL_Header + 'CI' + ci_str + termchar, motor_address)
	#sleep(0.1)
        #resp,addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'CC' + cc_str + termchar, motor_address)
	sleep(0.1)
        resp,addr = sock.recvfrom(1024)
	#sock.sendto(eSCL_Header + 'MO9' + termchar, motor_address) # Enable motion bit Y2
	#sleep(0.1)
        #resp,addr = sock.recvfrom(1024)
	#sock.sendto(eSCL_Header + 'JD' + termchar, motor_address)
        #sleep(0.1)
        #resp,addr = sock.recvfrom(1024)
	vel_settings = 'AC' + accel_str + ', DE' + decel_str + ', VE' + vel_str
	logfile.write(datetime.now().isoformat(' ') + ': ' + vel_settings + '\n')
	return vel_settings

def startup_setup():
	sock.sendto(eSCL_Header + 'IFD' + termchar, motor_address)
	sleep(0.1)
        response, addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'CC' + termchar, motor_address) #Get current
	sleep(0.1)
        current, addr = sock.recvfrom(1024)
        #response, addr = sock.recvfrom(1024)
        #try:
        #        current = response.split("=")[1]
        #except IndexError:
        #        print response
	print "Allowing dagger to fall!"
	sock.sendto(eSCL_Header + 'CC0' + termchar, motor_address)   #Set current to 0 (allows dagger to fall to hard stop gently)
	lastPos, addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'MD' + termchar, motor_address)   #Set current to 0 (allows dagger to fall to hard stop gently)
	lastPos, addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'IE' + termchar, motor_address)   #Get encoder position
	lastPos, addr = sock.recvfrom(1024)
	while True:
		print lastPos
		sleep(1.0)  #poll every 1s
		sock.sendto(eSCL_Header + 'IE' + termchar, motor_address) #Get next encoder position
		curPos, addr = sock.recvfrom(1024)
		if curPos == lastPos: #If it's the same twice the dagger is on hard stop
			break
		lastPos = curPos
	sock.sendto(eSCL_Header + 'EP0' + termchar, motor_address) #Zero Encoder
        resp, addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'SP0' + termchar, motor_address) #Zero current position
        resp, addr = sock.recvfrom(1024)
	print "Setting Stall Prevention mode!"
	print "Current: " + current
        sock.sendto(eSCL_Header + 'ME' + termchar, motor_address)   #Set current to 0 (allows dagger to fall to hard stop gently)
        resp, addr = sock.recvfrom(1024)
        sock.sendto(eSCL_Header + 'CC2' + termchar, motor_address) #Max out current before enabling stall prevention
	sleep(0.1)
        resp, addr = sock.recvfrom(1024)
	#sock.sendto(eSCL_Header + 'CI0.5' + termchar, motor_address)
	#sleep(1.0)
        #resp, addr = sock.recvfrom(1024)
	sock.sendto(eSCL_Header + 'EF6' + termchar, motor_address)
	sleep(1.0)
        resp, addr = sock.recvfrom(1024)
	#sock.sendto(eSCL_Header + 'CC' + current + termchar, motor_address)
	
	set_motor_params()

try:
	if TESTMODE:
		status = 'SC=0001\n'
	else:
                sock.sendto(eSCL_Header + 'SC' + termchar, motor_address)  # Request status word from the motor controller
                status, addr = sock.recvfrom(1024)
                #print status
                #sock.sendto(eSCL_Header + 'SC' + termchar, motor_address)  # Request status word from the motor controller
                #status, addr = sock.recvfrom(1024)
                #sock.sendto(eSCL_Header + 'SC' + termchar, motor_address)  # Request status word from the motor controller
                #status, addr = sock.recvfrom(1024)
                #sock.sendto(eSCL_Header + 'SC' + termchar, motor_address)  # Request status word from the motor controller
                #status, addr = sock.recvfrom(1024)
                #sock.sendto(eSCL_Header + 'SC' + termchar, motor_address)  # Request status word from the motor controller
                #status, addr = sock.recvfrom(1024)
                print status

	print 'Status reply from motor controller: ' + status
	if status[2:5] == 'SC=':
		logfile.write(datetime.now().isoformat(' ') + ': Motor controller status: ' + status + '\n')
		print 'Motor controller status: ' + status
	else:
		print 'No response from motor controller. Exiting.'
		sys.exit()

	startup_setup()

	# Setup the ZeroMQ objects
	context = zmq.Context()
	socket = context.socket(zmq.REP)
	socket.bind('ipc:///tmp/zeromq_dagger')#_pulseblaster')

	# Listen for commands over the wire.
	print 'Listening...'
	while True:
		command = socket.recv()
		if command[0:5] == 'SET Z':
			#sock.sendto(eSCL_Header + 'IE' + termchar, motor_address)
			z_request = string.atoi(command[5:])
			if z_request < z_min:
				z_request = z_min
				logstring = datetime.now().isoformat(' ') + ': Requested position < z_min (' + str(z_min) + '), setting to z_min.'
				print logstring
				logfile.write(logstring + '\n')
			if z_request > z_max:
				z_request = z_max
				logstring = datetime.now().isoformat(' ') + ': Requested position > z_max (' + str(z_max) + '), setting to z_max.'
				print logstring
				logfile.write(logstring + '\n')
			# Note: upward movement is negative!
			motor_command = 'FP' + str(-1*z_request)
			sock.sendto(eSCL_Header + motor_command + termchar, motor_address)
                        resp,addr = sock.recvfrom(1024)
			logstring = datetime.now().isoformat(' ') + ': ' + command + ', (' + motor_command + ')'
			print logstring
			logfile.write(logstring + '\n')
			reply_str = 'Z ' + str(z_request)
			socket.send(reply_str)
		elif command[0:7] == 'QUEUE Z':
			z_request = string.atoi(command[7:])
			if z_request < z_min:
				z_request = z_min
				logstring = datetime.now().isoformat(' ') + ': Requested position < z_min (' + str(z_min) + '), setting to z_min.'
				print logstring
				logfile.write(logstring + '\n')
			if z_request > z_max:
				z_request = z_max
				logstring = datetime.now().isoformat(' ') + ': Requested position > z_max (' + str(z_max) + '), setting to z_max.'
				print logstring
				logfile.write(logstring + '\n')
			# Note: upward movement is negative!
			motor_command = 'FP' + str(-1*z_request)
			sock.sendto(eSCL_Header + 'WI1F' + termchar, motor_address)
			sleep(0.1)
			#sock.sendto(eSCL_Headermotor_command + termchar, motor_address)
			sock.sendto(eSCL_Header + motor_command + termchar, motor_address)			
			logstring = datetime.now().isoformat(' ') + ': ' + command + ', (WI1L; ' + motor_command + ')'
			print logstring
			logfile.write(logstring + '\n')
			reply_str = 'QUEUED Z ' + str(z_request)
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMIN':
			reply_str = 'ZMIN ' + str(z_min)
			socket.send(reply_str)
		elif command[0:8] == 'GET ZMAX':
			reply_str = 'ZMAX ' + str(z_max)
			socket.send(reply_str)
			#print 'Setting AC='+accel_str + ', DE='+decel_str + ', VE=' + vel_str
		elif command[0:5] == 'GET Z':
			sock.sendto(eSCL_Header + 'IP' + termchar, motor_address)
                        sleep(0.1)
			resp,addr = sock.recvfrom(1024)
                        print resp
			#while '%' in resp or '?' in resp:
                        while 'IP' not in resp:
                                resp,addr = sock.recvfrom(1024)
                                print 'trying again...'
			#while '?' in resp:
                        #        resp,addr = sock.recvfrom(1024)
                        #        print 'trying again...'
                        reply_str = 'Z ' + resp.split('=')[1]
                        print ' '.join(resp.split('='))
			socket.send(reply_str)
		elif command[0:9] == 'RESET VEL':
			# Set the motor acceleration, deceleration, and velocity
			vel_settings = set_motor_params()
			socket.send(vel_settings)
		elif command[0:4] == 'ABRT':
			sock.sendto(eSCL_Header + 'SKD' + termchar, motor_address)
			logstring = datetime.now().isoformat(' ') + ': Aborted Queue and Stopped Movement'
			logfile.write(logstring + '\n')
			socket.send('ABORTED')
		else:
			logstring = datetime.now().isoformat(' ') + ': Command not recognized [' + command + ']'
			print logstring
			logfile.write(logstring + '\n')
			socket.send('COMMAND NOT RECOGNIZED')


finally:
	logfile.write(datetime.now().isoformat(' ') + ': Server stopped.\n')
	sock.close()
	logfile.close()

