import csv as csv
import time
import random
import serial
import datetime

#REMEMBER TO CLOSE THE COM PORT IN THE ARDUINO IDE
#Problems with access prevents both from being opened.

file = open('Template1.csv', 'w', newline='')
writer = csv.writer(file,delimiter = ',')
ITERATIONS = 1800
#time.sleep(3)
ser = serial.Serial(
    port='COM6',
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

header = ["200-A-Real","200-B-Real","200-A-Reactive","200-B-Reactive",\
"100-A-Real","100-B-Real","100-C-Real","100-A-Reactive","100-B-Reactive",\
"100-C-Reactive","5-A-Real","5-B-Real","5-C-Real","5-A-Reactive","5-B-Reactive",\
"5-C-Reactive"]
header = ["Time"]+header
writer.writerow(header)

header = ["N/A","Fridge-Real","N/A","Fridge-Reactive",\
"Disposal-Real","Microwave-Real","Dishwasher-Real","Disposal-Reactive","Microwave-Reactive",\
"Dishwasher-Reactive","N/A","N/A","N/A","N/A","N/A",\
"N/A"]
header = ["Appliance"]+header
writer.writerow(header)

while counter>0:
	data = ser.readline().decode('utf-8')
	if(len(data)>0):
		time_s = datetime.datetime.now()
		time = str(time_s.hour)+":"+str(time_s.minute)+":"+str(time_s.second)+"."+str(time_s.microsecond)
		#print(data)
		#process data
		data = data.split(',') #returns a list in the format [real power, reactive power]
		package = [" "+time] + data
		print(package)
		writer.writerow(package)
	counter-=1
	print("counter " + str(counter))
	if(counter==ITERATIONS/2):
		print("half way there")
	if(counter==ITERATIONS/4):
		print("3/4 way there")
	
print("DONE")	
        
file.close()
ser.close()