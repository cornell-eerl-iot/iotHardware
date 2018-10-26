import time
import serial

with serial.Serial(
	port='/dev/serial0',
	baudrate=19200,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=1
) as ser:
	try:
		while True:
			ser.write("a".encode('utf-8'))
	except KeyboardInterrupt:
		print "exited"
	
