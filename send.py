import time
import serial

ser=serial.Serial(
	port='/dev/ttyUSB0',
	baudrate=9600,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=1
)

message="4!hi"
ser.write(message)
time.sleep(5)

line= ''
while True:
	ser.write(message)
	lineNew=ser.readLine()
	if(line!=lineNew):
		print(lineNew)

ser.close()