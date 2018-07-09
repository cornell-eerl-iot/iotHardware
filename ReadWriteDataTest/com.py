import csv as csv
import time
import random
import serial
import datetime

file= open('testData.csv', 'a')

ser = serial.Serial(
    port='COM9',
    baudrate=19200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
    )
	
real_p = ''
reac_p = ''
mux = True

while True:
	display = real_p if(mux) else reac_p
	time_s = datetime.datetime.now()
	
	oneByte = ser.read(1)
	if(oneByte==" "):
		if(mux):
			time = str(time_s.month)+'/'+str(time_s.day)+'/'+str(time_s.year)+' '+str(time_s.hour)+':'+str(time_s.minute)+':'+str(time_s.second)
			file.write('\n' + time )
		file.write(","+display)
		print('\n' + time + str(display))
		real_p = ''
		reac_p = ''
		mux = not mux
	else:
		display += oneByte.decode("ascii")
	
	
	#if(lineNew != lineOld):
	

        
file.close()
ser.close()