
import meter_func
import threading
import time
import sys
import subprocess
import logging
import math
from meter_settings import *

class SerialMonitor(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        while True:
            try:
                meter_func.serial_monitor(True)
            except:
                print "error at Serial for FeatherM0"
                print "Unexpected error:", sys.exc_info()
                logging.error(str(time.localtime()) +"SERIALMONITOR()"+ str(sys.exc_info()))
            time.sleep(1)

class MeterMonitor(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        while True:
            try:
                interval = int(math.floor(MAX_MSG_SIZE/(READS_PER_PHASE*BYTE_SIZE_PER_READ*PHASE)))
                print ("Interval: " + str(interval))
                port = subprocess.check_output("ls /dev/ttyUSB*", shell=True) 
                port = port[:(len(port)-1)]
                meter_func.run_meter(port,interval,PHASE,addrs,BAUD_RATE,debug=True)
            except:
                print "error at Serial for Wattnode"
                print "Unexpected error:", sys.exc_info()
                logging.error(str(time.localtime()) +" METERMONITOR() "+ str(sys.exc_info()))
            time.sleep(1)



if __name__ == "__main__":
    logging.basicConfig(filename='/home/pi/iotHardware/error.log'+str(time.localtime()),filemode='w',level=logging.ERROR)
    Meter = MeterMonitor()
    Serial = SerialMonitor()
    # Gets the serial port for the USB modbus converter
    port = subprocess.check_output("ls /dev/ttyUSB*", shell=True) 
    port = port[:(len(port)-1)]
    meter_func.meter_init(port,BAUD_RATE,CT_SIZE_A,CT_SIZE_B,CT_SIZE_C,DIR_A,DIR_B,DIR_C)
    addrs = []
    for i in range(len(REG_ADDRS)):
        addrs.append([REG_ADDRS[i][0],PHASE*REGS_PER_READING,REG_ADDRS[i][1]])

    try:
        
        Meter.start()
        time.sleep(1)
        Serial.start()
        while True:
            if not Meter.is_alive():
                Meter = MeterMonitor()
                Meter.start()
            if not Serial.is_alive():
                Serial = SerialMonitor()
                Serial.start()
            time.sleep(1)

    except:
        print "undefined stop"
