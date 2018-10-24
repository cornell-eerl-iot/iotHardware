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

message=[0,1,2,3,4,5,6,7,8,9]

try:
    while(1):
        #instr = ser.read(1)
        #print "reading: " + repr(instr)
        
        #if(instr == '1'):
        print "writing.."
        print ser.write(message)
        time.sleep(0.2)
        for i in range(12):
            resp = ser.read()
            print repr(resp)
except:
    print "disconnected"

ser.close()