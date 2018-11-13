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

ser_connection = 1

def run_meter():
    """
    packs seconds of data 
    """
    
    try:
        print "starting"

        SERIAL = '/dev/ttyUSB0'
        BAUD = 19200

        client =  modbus.ModbusSerialClient(method='rtu', port=SERIAL,\
        stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')

        connection = client.connect()
        print "Readout started"

        time.sleep(1)
        
        msg_size = 7
        while(connection):
            packed  =  []
            message = []
            message.append(time.localtime()[4]) #local relative hour
            message.append(time.localtime()[5]) #local relative minutes
            for i in range(msg_size):
                response = client.read_holding_registers(1700,count = 2,unit = 1)
                #print "Getting response"
                output = (response.registers[0])|(response.registers[1]<<16)
                #compressed = fcomp.compress(output)
                message.append((output&0xff000000) >> 24)
                message.append((output&0x00ff0000) >> 16)
                message.append((output&0x0000ff00) >> 8)
                message.append((output&0x000000ff) >> 0)
                #processed = struct.unpack('f', struct.pack('I',output))
                #packed.append(struct.pack('>I',compressed).encode('hex'))
                
                time.sleep(0.98) #delay to account for computation time
            for mes in message:
                packed.append(struct.pack('>B',mes).encode('hex'))
            Queue.append(packed)
            print "message = " + repr(message) 
            
    except KeyboardInterrupt:
        print "disconnected"
    client.close()


def serial_monitor():
    with serial.Serial(
        port='/dev/serial0',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
    ) as ser:
        while ser_connection:
            if len(Queue) != 0 and ser.read() == '<':
                ser.write('>')
                ser.reset_input_buffer()
                msg = Queue.popleft()
                print "msg = " + repr(msg)
                for p in msg:
                    ser.write(p)