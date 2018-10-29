import time
import serial

with serial.Serial(
	port="COM7",#'/dev/serial0',
	baudrate=19200,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=2
) as ser:
		

	try:
		while(1):
			a = ser.read()
			print "reading: " + repr(a)
		   #time.sleep(1)
	except KeyboardInterrupt:
		print "disconnected"

