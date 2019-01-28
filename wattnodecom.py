# use ls /dev/tty* for list of ports
# Modbus Frame for Holding Registers (3)
# Send Frame: 
#   [slaveAddr, Func, addr_1st_Reg, num_requested, CRC]
# Recieve Frame:
#    [slaveAddr, Func, number of bytes, reg1 data, reg2 data, .... reg_n data, CRC]

import time
import pymodbus.client 
from pymodbus.client import sync as modbus
import pymodbus.register_read_message
import logging
import sys
import halfprecisionfloat
import struct

'''
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)
'''
time_limit = 10
start = time.time()

SERIAL = '/dev/ttyUSB1'
BAUD = 19200

client =  modbus.ModbusSerialClient(method='rtu', port=SERIAL,\
 stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')

connection = client.connect()
print "Readout started"

try:
    while(connection):
        #for i in range(msg_size):
        start = time.time()
        print "getting resp"
        response = client.read_holding_registers(1148,count = 14,unit = 1)
        time.sleep(0.5)
        response2 = client.read_holding_registers(1010,count = 6,unit = 1)

        responses = []
        i = 0
        j = 0
        print len(response.registers)
        while j<len(response2.registers)-1:
            raw = (response2.registers[j]) | (response2.registers[j+1]<<16)
            j+=2
            processed = struct.unpack('f', struct.pack('I',raw))
            responses.append(processed)

        while i<len(response.registers)-1:
            raw = (response.registers[i]) | (response.registers[i+1]<<16)
            i+=2
            processed = struct.unpack('f', struct.pack('I',raw))
            responses.append(processed)

        #while i<len(response.registers):
        #    responses.append(response.registers[i])
        #    i+=1
        
        print responses    
        #output1 = (response.registers[0])|(response.registers[1]<<16)
        #output2 = (response.registers[2])|(response.registers[3]<<16)
        #output3 = (response.registers[4])|(response.registers[5]<<16)
        #processed1 = struct.unpack('f', struct.pack('I',output1))
        #processed2 = struct.unpack('f', struct.pack('I',output2))
        #processed3 = struct.unpack('f', struct.pack('I',output3))
        #msg.append(output)
        #print (processed1, processed2, processed3)                
        if(time.time()-start)>time_limit:
            connection = False
        end = time.time()
        delay = max(0,1- end+start)
        print "time diff: " + repr(end-start)
        time.sleep(delay)
    
        #print sys.getsizeof(msg)
except KeyboardInterrupt:
    print "closing"
client.close()

