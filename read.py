import time
import serial

ser=serial.Serial(
	port='COM6',
	baudrate=19200,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=1
)

try:
    while(1):
        a = ser.read(1)
        print "reading: " + repr(a)
        time.sleep(1)
except:
    print "disconnected"

ser.close()