import serial
import time
import pymodbus.client 
from pymodbus.client import sync as modbus
import pymodbus.register_read_message
import logging
import sys
import halfprecisionfloat
import struct
from collections import deque

fcomp = halfprecisionfloat.Float16Compressor()
lst = []
Queue = deque(lst,5)
connection = 0

def meter_init(A=100,B=100,C=100,a=0,b=0,c=0):
    """
    Initializes the meter settings
    A,B,C: CT ratings for phase A,B,C respectively
    a,b,c: CT directions. 0 or 1
    """  
    SERIAL = '/dev/ttyUSB0'
    BAUD = 19200

    client =  modbus.ModbusSerialClient(method='rtu', port=SERIAL,\
        stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')

    connection = client.connect()
    print ("initializing meter, connection = " +str(connection) )
    time.sleep(1)
    
    print ("Reading CT settings")
    response = client.read_holding_registers(1602,count=4,unit=1)
    print ("%d, %d, %d, %d" %(response.registers[0], response.registers[1],\
            response.registers[2], response.registers[3]))
    
    print ("Writing to CT registers")
    client.write_registers(1603, [A,B,C])
    print ("Reading CT directions")
    response = client.read_holding_registers(1606,count=1,unit=1)
    print ("%d" %(response.registers[0]))

    print ("setting CT's")
    value = a|(b<<1)|(c<<2)
    client.write_registers(1606,value)


    client.close()


def run_meter(PORT, BAUD=19200, ITERATIONS=0):
    """
    packs seconds of data 
    """
    
    try:
        print "starting"

        #SERIAL = '/dev/ttyUSB0'
        #BAUD = 19200

        client =  modbus.ModbusSerialClient(method='rtu', port=PORT,\
        stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')
        global connection
        connection = client.connect()
        print "connection is "+ str(connection)

        time.sleep(1)
        
        msg_size = 5
        q = 0
        while(connection and (ITERATIONS==0 or q<ITERATIONS)):
            q+=1
            packed  = []
            message = []
            message.append(time.localtime()[4]) #local relative minutes
            message.append(time.localtime()[5]) #local relative seconds
            addrs = [[1010,6,1],[1148,6,1]]
            for i in range(msg_size):
                start_time = time.time()
                #print "Polling Response"
                #Read registers 1010 - 1016 real power
                #1148  - 1153 reactive power
                # response = client.read_holding_registers(1700,count = 2,unit = 1)
                # output = (response.registers[0])|(response.registers[1]<<16)
                # message.append((output&0xff000000) >> 24)
                # message.append((output&0x00ff0000) >> 16)
                # message.append((output&0x0000ff00) >> 8)
                # message.append((output&0x000000ff) >> 0)

                response = client.read_holding_registers(1010,count = 6,unit = 1)
                phaseAreal = (response.registers[0])|(response.registers[1]<<16)
                phaseBreal = (response.registers[2])|(response.registers[3]<<16)
                phaseCreal = (response.registers[4])|(response.registers[5]<<16)
                
                phaseArealcomp = fcomp.compress(phaseAreal)
                phaseBrealcomp = fcomp.compress(phaseBreal)
                phaseCrealcomp = fcomp.compress(phaseCreal)
                phases = [phaseArealcomp,phaseBrealcomp,phaseCrealcomp]
                for j in phases:
                    message.append(j & 0xff)
                    message.append((j & 0xff00)>>8)

                response = client.read_holding_registers(1148,count = 6,unit = 1)
                phaseAreactive = (response.registers[0])|(response.registers[1]<<16)
                phaseBreactive = (response.registers[2])|(response.registers[3]<<16)
                phaseCreactive = (response.registers[4])|(response.registers[5]<<16)
                
                phaseAreactivecomp = fcomp.compress(phaseAreactive)
                phaseBreactivecomp = fcomp.compress(phaseBreactive)
                phaseCreactivecomp = fcomp.compress(phaseCreactive)
                phases = [phaseAreactivecomp,phaseBreactivecomp,phaseCreactivecomp]
                
                for j in phases:
                    message.append(j & 0xff)
                    message.append((j & 0xff00)>>8)
                # #print "Got Response"
                # for j in range(12):
                #     message.append((i<<4)|j & 0xFF)
                end_time = time.time()
                print "time diff: " + repr(end_time-start_time)
                
                delay  = max(0, 1 - (end_time-start_time))
                time.sleep(delay) #delay to account for computation time
            for mes in message:
                packed.append(struct.pack('>B',mes).encode('hex'))
                
            Queue.append(packed)
            #print "message = " + repr(message) 
            
    except:
        print "meter disconnected"    
    client.close()


def serial_monitor():
    try:
        with serial.Serial(
            port='/dev/serial0',
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        ) as ser:
            while True:
                if len(Queue)>0:
                    print "Queue: " + repr(Queue)
                if len(Queue) != 0 and ser.read() == '<':
                    ser.write('>')
                    print "got ready signal!"
                    msg = Queue.popleft()
                    print (msg)
                    for p in msg:
                        ser.write(p)
                    ser.reset_input_buffer()
                else:
                    time.sleep(0.1) #SUPPER IMPORTANT AS TO NOT OVERLOAD CPU
                    
    except:
        print "serial error"
        global connection
        connection  = 0



if __name__=="__main__":
   meter_init(100,100,200,0,0,0)
   #run_meter()
