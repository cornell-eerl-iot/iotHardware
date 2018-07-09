import csv as csv
import time
import random
import serial
import datetime

file = open('testData.csv', 'w', newline='')
writer = csv.writer(file,delimiter = ',')
ITERATIONS = 100
#time.sleep(3)
ser = serial.Serial(
    port='COM9',
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
    )
#COM will have identifier. The format will be
#     '##,##'
#First number is real, second is reactive

counter = ITERATIONS
print("starting program")
while counter>0:

	data = ser.readline().decode('utf-8')
	
	if(len(data)>0):
		
		time_s = datetime.datetime.now()
		time = str(time_s)
		
		
		#process data
		
		power = data.split(',') #returns a list in the format [real power, reactive power]
		print(power)
		if(len(power)>1):
			print([time,power[0],power[1].replace('\r\n','')])
			writer.writerow([time,power[0].replace('\r\n',''),power[1].replace('\r\n','')])
		
		
	counter-=1
	
	
	

        
file.close()
ser.close()