#!/usr/bin/python

import sys
import math
from time import sleep, time
import socket
import mysql.connector
from mysql.connector import errorcode

class Mensor:
	'''An object to handle communication with objects up on the mezzanine: namely the lakeshores, d2 mensor, and level meter.
	They're all in one object since they share the same GPIB bus.'''
	def __init__(self, dbname):
		self.cnx = mysql.connector.connect(user='ucn', password='slmmmtqom')
		self.cursor = self.cnx.cursor()

		# Connect to the database
		try:
		    self.cnx.database = dbname
		except mysql.connector.Error as err:
		    if err.errno == errorcode.ER_BAD_DB_ERROR:
		        print('You need to create the database first. See README.')
		    print(err)
		    exit(1)

		self.cursor = self.cnx.cursor()
		
		self.msock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP) #communication is handled via socket using GPIB/ethernet converter
		#self.msock.setblocking(0)
		self.msock.connect(('UCN-PROLOGIX7.lanl.gov',1234))

		#Address mensor
		self.msock.send("++addr 11\n")
		#Tell mensor to clear, also puts into remote mode
		self.msock.send("DCL\n")
		#Read anything left and discard
		resp = self.msock.recv(1024)

	def __del__(self):
		self.msock.close()
		self.cursor.close()
		self.cnx.close()
		
	def scan(self):  #I want to average pressures since the mensor does ~40 measurements every 5 seconds. Excessive for EMS monitoring
		#Start with empty list of pressures
		measurements = []
		tStart = time()
		#Get data for 5 seconds
		while time() - tStart <5.0:
			#Take what the mensor gives
			resp = self.msock.recv(1024)
			pressure  = None
			#Try and parse as float. If not possible, give up.
			try:
				pressure = float(resp)
			except ValueError: #If we can't parse a value, then continue with next
				continue
			measurements.append(pressure)
		avgP = None

		if len(measurements) > 0:
			avgP = reduce(lambda sum, p: sum+p, measurements, 0.0)/len(measurements)
			ddl = "INSERT INTO Data_Mensor SET"
	                ddl += " unixtime=UNIX_TIMESTAMP(NOW())"
        	        ddl += ", time=NOW()"
                	ddl += ", instrumentID=11"
	                ddl += ", channel=1"
        	        ddl += ", P=" + str(avgP)
                	try:
	                    print(ddl)
        	            self.cnx.ping(reconnect=True, attempts=5, delay=0)
                	    self.cursor.execute(ddl)
	                    self.cnx.commit()
        	        except mysql.connector.Error as err:
                	    print(err.msg)
	                else:
        	            print("OK")

dbname = None
try:
    dbname = sys.argv[1]
except:
    print('Usage: python read_flexrax.py [dbname]')
    exit(1)

men = Mensor(dbname)
while(True):
	men.scan()
