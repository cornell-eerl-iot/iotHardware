
import meter_func
import threading
import time
import sys

class SerialMonitor(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        meter_func.serial_monitor()

class MeterMonitor(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        meter_func.run_meter()a



if __name__ == "__main__":
    Meter = MeterMonitor()
    Serial = SerialMonitor()
    
    try:
        while True:
            try:
                if not Meter.is_alive():
                    Meter.start()
                time.sleep(1)
                if not Serial.is_alive():
                    Serial.start()
                
            except:
                print "Unexpected error:", sys.exc_info()
                Meter = MeterMonitor()
                Serial = SerialMonitor()
                
            time.sleep(1)
            
    except:
        print "undefined stop"