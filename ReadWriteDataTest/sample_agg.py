"""
This module reads data sent over serial and writes to an Excel file

"""

import csv as csv
import time
import random
import serial
import datetime

#REMEMBER TO CLOSE THE COM PORT IN THE ARDUINO IDE
#Problems with access prevents both from being opened.

file = open('Template1.csv', 'w', newline='')
writer = csv.writer(file,delimiter = ',')

ITERATIONS = 1800 #Choose time

ser = serial.Serial(
    port='COM6', #Will need to adjust depending on USB port / OS
    baudrate=19200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=5
    )
#COM will have identifier. The format will be
#     '##,##'
#First number is real, second is reactive

counter = ITERATIONS
print("starting program")

header = ["200-A-Real","200-B-Real","200-C-Real","200-A-Reactive","200-B-Reactive","200-C-Reactive",\
"100-A-Real","100-B-Real","100-C-Real","100-A-Reactive","100-B-Reactive","100-C-Reactive"]
header = ["Time"]+header
writer.writerow(header)
header = ["Grid Power-A-Real","Grid Power-B-Real","N/A","Grid Power-A-Reactive","Grid Power-B-Reactive",\
"N/A","AHU-A-Real","AHU/RTU-B-Real","RTU-A-Real","AHU-A-Reactive","AHU/RTU-B-Reactive",\
"RTU-Reactive"]
header = ["Time"]+header
writer.writerow(header)

while counter>0:
	data = ser.readline().decode('utf-8')
	if(len(data)>0):
		time_s = datetime.datetime.now() 
		#Opted to manually set how time should be displayed so that we can display relevant
		#information
		time = str(time_s.hour)+":"+str(time_s.minute)+":"+str(time_s.second)+"."+str(time_s.microsecond)
		#process data
		data = data.split(',') #returns a list in the format [real power, reactive power]
		package = [" "+time] + data
		print(package)
		writer.writerow(package)
	counter-=1
	print("counter " + str(counter))
	
print("DONE")	
        
file.close()
ser.close()