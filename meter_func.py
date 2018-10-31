import serial
import time
import pymodbus.client 
from pymodbus.client import sync as modbus
import pymodbus.register_read_message
import logging
import sys
import halfprecisionfloat
import struct

def run_meter():
    with serial.Serial(
        port='/dev/serial0',
        baudrate=19200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1
    ) as ser:
        try:
            message=[]
            packed =[]
            print "starting"

            SERIAL = '/dev/ttyUSB0'
            BAUD = 19200

            client =  modbus.ModbusSerialClient(method='rtu', port=SERIAL,\
            stopbits=1, bytesize=8, timeout=3, baudrate=BAUD, parity='N')

            connection = client.connect()
            print "Readout started"

            time.sleep(1)
            
            msg_size = 2
            while(connection):
                message = []
                packed =  []
                for i in range(msg_size):
                    response = client.read_holding_registers(1700,count = 2,unit = 1)
                    #print "Getting response"
                    output = (response.registers[0])|(response.registers[1]<<16)
                    #aa = bytearray(output)
                    #processed = struct.unpack('f', struct.pack('I',output))
                    message.append(output)
                                
                    time.sleep(1)
                            
                for mes in message:
                    packed.append(struct.pack('>I',mes).encode('hex'))
                while ser.read() != '<':
                    pass
                #print "signal from MCU: " + repr(a)
                #if(a=='<'):
                    #print ser.readline()
                print "message = " + repr(message)     
                print "packed = " + repr(packed)
                for p in packed:
                    ser.write(p)#.encode('utf-8'))
                print ser.readline()
                #print sys.getsizeof(msg)
        except:
            print "disconnected"
        client.close()