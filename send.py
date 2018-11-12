import time
import serial
import struct

with serial.Serial(
	port='COM6',#/dev/serial0',
	baudrate=19200,
	parity=serial.PARITY_NONE,
	stopbits=serial.STOPBITS_ONE,
	bytesize=serial.EIGHTBITS,
	timeout=1
) as ser:

    time.sleep(1)
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
            #pack the integers into 2 bytes
            for mes in message:
                packed.append(struct.pack('>H',mes).encode('hex'))
            print "start listening"
            if ser.read() == '<':
                ser.write('>')
                ser.reset_input_buffer()
                
                print "read '<'"
                print "message = " + repr(message)     
                print "packed = " + repr(packed)
                for p in packed:
                    ser.write(p)
                print ser.readline()

    except KeyboardInterrupt:
        print "disconnected"
