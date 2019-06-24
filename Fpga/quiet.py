#!/usr/bin/python

import time
import sys
import serial
import zmq    # ZeroMQ package. This may need to be installed.
import string
from datetime import datetime
from subprocess import call

TERM = "\n\r" # EOL for FPGA
TESTMODE = False # if True, commands are not actually sent to the FPGA
WAIT = 0.1 # delay after reading/writing serial port

logstring = ''

class Point:
    def __init__(self,t0,t1,out):
        self.ticks0 = t0
        self.ticks1 = t1
        self.fpga_out = out
    
    def __str__(self):
        return 'Point<ticks0={0},ticks1={1},fpga_out={2}>'.format(self.ticks0,self.ticks1,self.fpga_out)

# Read all characters on the serial port and return them
def serial_read(chunk_size=1000):
    if not ser.timeout: raise TypeError('Port needs to have a timeout set!')

    read_buffer = b''
    while True:
        byte_chunk = ser.read(size=chunk_size)
        read_buffer += byte_chunk
        if not len(byte_chunk) == chunk_size: break
	
    return read_buffer

def serial_write(message):
    ser.write((message+TERM).encode("utf-8"))
    time.sleep(WAIT)

def exit_server():
    print "Exiting server"
    if TESTMODE: testfile.close()
    print logstring
    logfile.close()
    sys.exit()



logfile = open('fpga.log', 'a')
logfile.write(datetime.now().isoformat(' ') + ': FPGA server started\n')

if TESTMODE:
    testfile = open('test_fpga.txt', 'a')

try:
    # Setup the ZeroMQ objects
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind('ipc:///tmp/zeromq_fpga')
    
    # setup serial connection to FPGA
    # edit port for your local machine
    if not TESTMODE:
	ser = serial.Serial(port='/dev/arty', baudrate=9600, parity='N', stopbits=1, bytesize=8, timeout=WAIT)

    ########## BEGIN HANDSHAKE ##########
    print "Slow Control says \"reset\" to FPGA"
    hello_str = "reset"
    if not TESTMODE:
        print serial_read() # clear buffer
        serial_write(hello_str)
        reply = serial_read().split(TERM)
    else:
        testfile.write(hello_str+'\n')
        reply = ['FPGA has reset!', '4294967295 83333333 13 6156'] # update if you change the FPGA input/output scheme
    
    if reply[0] != "FPGA has reset!":
	print reply[0]
        print "ERROR! FPGA didn't hear the \"reset\" correctly"
        print "Try the following 3-step process"
        print "\t1: physically go to the FPGA board inside of box, press PROG button (upper-left hand corner)"
        print "\t2: wait until a green LED turns on in the lower-left hand corner of the board"
        print "\t3: restart this script (sometimes this takes two attempts)"
        exit_server()
    
    print "FPGA replies to Slow Control's \"reset\" message"
    line = reply[1].split()
    MAX_TICKS = int(line[0])+1 # +1 because FPGA sends 2^32 - 1
    TIMER_FREQ = int(line[1])
    PATTERN_LENGTH = int(line[2])
    INITIAL_STATE = int(line[3])
    
    pulse_info = socket.recv() # should be of the format "<PULSE_CHANNELS> <PULSE_LOW> <PULSE_TIME_MS>"
    logstring = datetime.now().isoformat(' ') + ': ' + 'sending pulse info from Slow Control to server to FPGA'
    vals = pulse_info.split()
    if len(vals)!=3:
        logstring+='\nERROR! Slow Control must send a string of three values: "<PULSE_CHANNELS> <PULSE_LOW> <PULSE_TIME_MS>"'
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    
    try:
        PULSE_CHANNELS = int(vals[0])
        PULSE_LOW = int(vals[1])
        PULSE_TIME_MS = int(vals[2])
    except:
        logstring += '\nERROR! The three values must be ints'
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
        
    if PULSE_CHANNELS<0 or PULSE_CHANNELS>=2**PATTERN_LENGTH:
        logstring += '\nERROR! Must have 0 <= PULSE_CHANNELS <= 2**PATTERN_LENGTH-1'
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    elif PULSE_LOW<0 or PULSE_LOW>=2**PATTERN_LENGTH:
        logstring += '\nERROR! Must have 0 <= PULSE_LOW <= 2**PATTERN_LENGTH-1'
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    elif PULSE_TIME_MS<=0 or PULSE_TIME_MS>1000:
        logstring += '\nERROR! Must have 0 ms < PULSE_TIME_MS <= 1000 ms'
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    
    serial_write('{0}'.format(PULSE_CHANNELS))
    if int(serial_read())!=PULSE_CHANNELS:
        logstring += "\nERROR! FPGA didn't hear the correct value for PULSE_CHANNELS"
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    serial_write('{0}'.format(PULSE_LOW))
    if int(serial_read())!=PULSE_LOW:
        logstring += "\nERROR! FPGA didn't hear the correct value for PULSE_LOW"
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
    serial_write('{0}'.format(PULSE_TIME_MS))
    if int(serial_read())!=PULSE_TIME_MS:
        logstring += "\nERROR! FPGA didn't hear the correct value for PULSE_TIME_MS"
        print logstring
        logfile.write(logstring + '\n')
        exit_server()
        
    socket.send("END HANDSHAKE")
    ########### END HANDSHAKE ###########

    # Listen for commands over the ZMQ socket
    print "Listening..."
    n = 0
    points = []
    while True:
        command = socket.recv()
        logstring = datetime.now().isoformat(' ') + ': '
        
        if command[0:10] == 'GET PARAMS':
            logstring += 'sending FPGA params to Slow Control'
            socket.send("MAX_TICKS {0} TIMER_FREQ {1} PATTERN_LENGTH {2} INITIAL_STATE {3}".format(MAX_TICKS,TIMER_FREQ,PATTERN_LENGTH,INITIAL_STATE))
        elif command[0:13] == "LOAD RUNFILE ":
            logstring += 'Slow Control sending run file'
            try:
                n = int(command[13:])
                if n<1:
                    logstring += '\nERROR! Must have n>=1, setting n=0, need to call RESET RUNFILE'
                    n=0
            except:
                logstring += '\nERROR! Couldnt convert to int, setting n=0, need to call RESET RUNFILE'
                n=0
            socket.send("N " + str(n))
            
            command = socket.recv()
            if command!="GOOD N":
                n=0
                socket.send("BAD N")
                continue
            else:
                socket.send("GOOD N")
            
            points = []
            for i in range(n): # 3 values (ticks0, ticks1, fpga_out) per point
                command = socket.recv()
                try:
                    line = command.split()
                    t0 = int(line[1])
                    t1 = int(line[3])
                    out = int(line[5])
                except:
                    logstring += '\nERROR! {0}th input wasnt three ints, setting to 0s, must call RESET RUNFILE'.format(i)
                    t0 = 0
                    t1 = 0
                    out = 0
                points.append(Point(t0,t1,out))
                socket.send("TICKS0 " + str(t0) + " TICKS1 " + str(t1) + " STATE " + str(out))
        elif command[0:13] == 'RESET RUNFILE':
            logstring += 'resetting the run file on the server'
            n = 0
            points = []
            socket.send('RESET RUNFILE DONE')
        elif command[0:9] == "LOAD FPGA":
            logstring += 'sending the run file from the server to the FPGA'
            fpga_loaded_correctly = True
            if n!=len(points):
                logstring += '\nERROR! Dont have the right number of data points loaded to the server, cant start loading to FPGA, must call RESET RUNFILE'
                socket.send('NOT OKAY TO LOAD FPGA')
            else:
                if not TESTMODE:
                    serial_write("prepare_for_run")
                    # after a successful run, serial_read() is padded in front with multiple "ERROR! Must enter \"prepare_for_run\" in order to receive\n\r"
                    # strip these away by splitting on TERM, and do [-2] to skip back past the empty string after the final TERM
                    x = serial_read().split(TERM)[-2]
                    if x!="FPGA ready to receive the run":
                        fpga_loaded_correctly = False
                    else:
                        serial_write(str(n))
                        if int(serial_read()) != n:
                            logstring += '\nERROR! FPGA got wrong n'
                            fpga_loaded_correctly = False
                else:
                    testfile.write(str(n)+'\n')
                if fpga_loaded_correctly: # n was received correctly
                    for i,p in enumerate(points):
                        if not TESTMODE:
                            serial_write(str(p.ticks0))
                            if int(serial_read()) != p.ticks0:
                                logstring += '\nERROR! FPGA got the wrong {0}th ticks0'.format(i)
                                fpga_loaded_correctly = False
                            serial_write(str(p.ticks1))
                            if int(serial_read()) != p.ticks1:
                                logstring += 'ERROR! FPGA got the wrong {0}th ticks1'.format(i)
                                fpga_loaded_correctly = False
                            serial_write(str(p.fpga_out))
                            if int(serial_read()) != p.fpga_out:
                                logstring += '\nERROR! FPGA got the wrong {0}th fpga_out'.format(i)
                                fpga_loaded_correctly = False
                        else:
                            testfile.write(str(p.ticks0)+'\n')
                            testfile.write(str(p.ticks1)+'\n')
                            testfile.write(str(p.fpga_out)+'\n')
                if fpga_loaded_correctly:
                    socket.send('FPGA LOADED GOOD')
                else:
                    socket.send('FPGA LOADED BAD')
        elif command[0:14] == 'START FPGA NOW':
            logstring += 'starting the FPGA now'
            if not TESTMODE: serial_write('start_now')
            else: testfile.write('start_now\n')
            socket.send('STARTED FPGA NOW')
        elif command[0:16] == 'START FPGA HMGX ':
            logstring += 'starting the FPGA HmGX'
            try:
                prestate = int(command[16:])
                if n<0:
                    logstring += '\nERROR! Must have prestate>=0, setting prestate=0, please do better next time'
                    prestate=0
            except:
                logstring += '\nERROR! Couldnt convert to int, setting prestate=0, please do better next time'
                prestate=0           
            if not TESTMODE:
                serial_write('start_HmGX')
                serial_write(str(prestate))
                while True:
                    line = serial_read()
                    if line[0:19]=='First_pos_edge_HmGX': # found first HmGX edge, move on
                        socket.send('FIRST POS EDGE HMGX')
                        socket.recv() # meaningles, KEEP GOING
                        while True:
                            line = serial_read()
                            if line[0:20]=='Second_pos_edge_HmGX': # found second HmGX, starting
                                socket.send('SECOND POS EDGE HMGX')
                                break
                            elif line[0:12]=='HmGX_timeout': # never found second HmGX edge, timed out
                                socket.send('HMGX TIMEOUT')
                                break
                            else: time.sleep(WAIT)
                        break # only happens after you find the first HmGX edge, and then time out or find the second HmGX edge
                    elif line[0:12]=='HmGX_timeout': # never found first HmGX edge, timed out
                        socket.send('HMGX TIMEOUT')
                        break
                    else: time.sleep(WAIT)
            else:
                testfile.write('start_HmGX\n')
                testfile.write(str(prestate)+'\n')
                time.sleep(5)
                socket.send('FIRST POS EDGE HMGX')
                socket.recv() # meaningless, KEEP GOING
                time.sleep(5)
                socket.send('SECOND POS EDGE HMGX')
        elif command[0:15] == 'DONT START FPGA':
            logstring += 'not starting the FPGA, load the file again'
            n = 0
            points = []
            if not TESTMODE: serial_write('dont_start')
            else: testfile.write('dont_start\n')
            socket.send('DIDNT START FPGA')
        elif command[0:26] == 'SLOW CONTROL AT END OF RUN':
            logstring += 'Slow Control signalling it is at the end or run, checking to see if FPGA is done\n'
            if not TESTMODE:
                done = False
                while True:
                    lines = serial_read().split(TERM)
                    for x in lines:
                        if x[0:3]!='Run': continue
                        elif x[0:17]=='Run_ran_correctly':
                            socket.send('RUN GOOD')
                            done = True
			    logstring += "FPGA signals the run ran correctly"
                            break
                        elif x[0:11]=='Run_aborted':
                            socket.send('RUN ABORTED')
                            done = True
			    logstring += "FPGA signals the run was aborted"
                            break
                        elif x[0:19]=='Run_ran_incorrectly':
                            socket.send('RUN BAD')
                            done = True
			    logstring += "FPGA signals the run did not run correctly"
                            break
                    if done: break
            else:
                socket.send("RUN GOOD")
        else:
            logstring += 'command not recognized [' + command + ']'
            socket.send('COMMAND NOT RECOGNIZED')
        
        print logstring
        logfile.write(logstring + '\n')
        
finally:
    exit_server()
