import csv as csv
import time
import random
import serial
import datetime

file = open('testData1.csv', 'w', newline='')
writer = csv.writer(file,delimiter = ',')
ITERATIONS = 10

ser = serial.Serial(
    port='COM9',
    baudrate=19200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
    )
#COM will have identifier. The format will be
#     '##,##'
#First number is real, second is reactive

counter = ITERATIONS
while counter>0:

	data = ser.readline().decode('utf-8')
	print(data)
	print(type(data))
	if(len(data)>0):
		
		time_s = datetime.datetime.now()
		time = str(time_s)
		
		 
		
		#process data
		
		power = data.split(',') #returns a list in the format [real power, reactive power]
		writer.writerow([time,power[0].replace('\r\n',''),power[1].replace('\r\n','')])
		print([time,power[0],power[1].replace('\r\n','')])
		print('')
	counter-=1
	
	

        
file.close()
ser.close()