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
import subprocess

fcomp = halfprecisionfloat.Float16Compressor()
lst = []
Queue = deque(lst,3)
connection = 0

def meter_init(PORT,BAUD=19200, A=100,B=100,C=100,a=0,b=0,c=0):
    """
    Initializes the meter settings
    A,B,C: CT ratings for phase A,B,C respectively
    a,b,c: CT directions. 0 or 1
    """  
    #SERIAL = '/dev/ttyUSB0'
    #BAUD = 19200

    client =  modbus.ModbusSerialClient(method='rtu', port=PORT,\
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

    print ("setting CT's directions")
    value = (a&0x1)|((b&0x1)<<1)|((c&0x1)<<2)
    client.write_registers(1606,value)


    client.close()


def run_meter(PORT, INTERVAL, PHASE, BAUD=19200, ITERATIONS=0, debug=True):
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
        time.sleep(0.5)
        interation_counter = 0
        num_regs_per_phase   = PHASE*2 #need to read real and reactive power regs
        package_length = INTERVAL*num_regs_per_phase*PHASE*2 #bytes/phase
        header_length  = 2
        msg_length    = package_length + header_length
        while(connection and (ITERATIONS==0 or interation_counter<ITERATIONS)):
            interation_counter+=1
            packed  = []
            message = []
            message.append(msg_length&0xFF)          #Doesn't count since it gets read
            message.append(time.localtime()[4]&0xFF) #local relative minutes
            message.append(time.localtime()[5]&0xFF) #local relative seconds
            addrs = [[1010,num_regs_per_phase,1],[1148,num_regs_per_phase,1]]
            for i in range(INTERVAL):
                start_time = time.time()
                #print "Polling Response"
                #Read registers 1010 - 1016 real power
                #1148  - 1153 reactive power
                for j in range(len(addrs)):
                    response = client.read_holding_registers(addrs[j][0],\
                    count = addrs[j][1],unit = addrs[j][2])
                    for k in range(0,len(response.registers),2):
                        val = (response.registers[k])|(response.registers[k+1]<<16)
                        valComp = fcomp.compress(val)
                        message.append(valComp & 0xff)
                        message.append((valComp & 0xff00)>>8)
                
                end_time = time.time()
                delay  = max(0, 1 - (end_time-start_time)) # how long needed to 
                # wait for next polling
                if debug:
                    print "time diff: " + repr(end_time-start_time)
                time.sleep(delay) #delay to account for computation time
            for mes in message:
                packed.append(struct.pack('>B',mes).encode('hex'))
                
            Queue.append(packed)
            if debug:
                print "len = " +str(len(message))+  " message = " + repr(message) 

            
    except Exception as e:
        print (e)
        print (str(e.message))
        print "meter disconnected"   
        logging.error(str(time.localtime()) + str(e.message))
    client.close()


def serial_monitor(debug=True):
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
                if len(Queue)>0 and debug:
                    print "Queue: " + repr(Queue)
                if len(Queue) != 0 and ser.read() == '<':
                    ser.write('>')

                    msg = Queue.popleft()
                    if debug:
                        print "got ready signal!"
                    for p in msg:
                        ser.write(p)
                        time.sleep(0.01)
                        
                    ser.reset_input_buffer()
                else:
                    time.sleep(0.1) #SUPPER IMPORTANT AS TO NOT OVERLOAD CPU
                    
    except Exception as e:
        print "serial error"
        logging.error(str(time.localtime())+ str(e.message))
        
      



if __name__=="__main__":
    while True:
        try:    
            port = subprocess.check_output("ls /dev/ttyUSB*", shell=True) 
            port = port[:(len(port)-1)]
            meter_init(port,19200,100,100,200,0,1,0)
            run_meter(port,8,2,ITERATIONS=3)
        except:
            print("exit")
    
    print ("DONE")
