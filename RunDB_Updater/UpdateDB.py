#!/usr/bin/python
import mysql.connector

cnx = mysql.connector.connect(user='daq', password='iucf1234')
cnx.close()
