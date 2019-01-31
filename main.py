
import meter_func
import threading
import time
import sys
import subprocess
import logging

INTERVAL = 8  #in seconds
BAUD_RATE = 19200

class SerialMonitor(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        while True:
            try:
                meter_func.serial_monitor()
            except:
                print "error at Serial for FeatherM0"
                print "Unexpected error:", sys.exc_info()
                logging.debug(sys.exc_info())
            time.sleep(1)

class MeterMonitor(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        while True:
            try:
                port = subprocess.check_output("ls /dev/ttyUSB*", shell=True) 
                port = port[:(len(port)-1)]
                meter_func.run_meter(port,INTERVAL,BAUD_RATE)
            except:
                print "error at Serial for Wattnode"
                print "Unexpected error:", sys.exc_info()
                logging.debug(sys.exc_info())
            time.sleep(1)



if __name__ == "__main__":
    logging.basicConfig(filename='/home/pi/iotHardware/error.log',filemode='w',level=logging.DEBUG)
    Meter = MeterMonitor()
    Serial = SerialMonitor()
    port = subprocess.check_output("ls /dev/ttyUSB*", shell=True) 
    port = port[:(len(port)-1)]
    #meter_func.meter_init(port,BAUD_RATE,100,100,200,0,1,0)
    try:
        
        Meter.start()
        time.sleep(0.5)
        Serial.start()
        while True:
            time.sleep(0.5)

    except:
        print "undefined stop"
