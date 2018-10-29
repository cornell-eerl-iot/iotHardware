import time
import serial
import struct

ser=serial.Serial(
	port='/dev/serial0',
	baudrate=19200,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=1
)

message=[]
packed =[]
print "starting"
try:
    while(1):
        #instr = ser.read(1)
        #print "reading: " + repr(instr)
        message = []
        packed =  []
        message.append(int(time.time()) &0xffff)
        time.sleep(1)
        message.append(int(time.time()) &0xffff)
        time.sleep(1)
        message.append(int(time.time()) &0xffff)
        for mes in message:
            packed.append(struct.pack('>H',mes).encode('hex'))
        print "start listening"
        a = ser.read()
        print "signal from MCU: " + repr(a)
        if(a=='<'):
            #print ser.readline()
            print "message = " + repr(message)     
            print "packed = " + repr(packed)
            for p in packed:
                ser.write(p)#.encode('utf-8'))
            print ser.readline()

except KeyboardInterrupt:
    print "disconnected"

ser.close()
