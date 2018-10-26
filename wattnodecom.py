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

start = time.time()
time_limit = 10

SERIAL = '/dev/ttyUSB0'
BAUD = 19200

client =  modbus.ModbusSerialClient(method='rtu', port=SERIAL,\
 stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')

connection = client.connect()
print "Readout started"

msg = []
msg_size = 2
try:
    while(connection):
        for i in range(msg_size):
            response = client.read_holding_registers(1010,count = 2,unit = 1)
            #print "Getting response"
            output = (response.registers[0])|(response.registers[1]<<16)
            #aa = bytearray(output)
            processed = struct.unpack('f', struct.pack('I',output))
            msg.append(processed)
                        
            if(time.time()-start)>time_limit:
                connection = False
            time.sleep(1)
        print msg
        #print sys.getsizeof(msg)
        msg = []
except KeyboardInterrupt:
    print "closing"
client.close()

