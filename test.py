
#import meter_func
import threading
import time
class SerialMonitor(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        # meter_func.serial_monitor()
        for i in range(10):
            print ("Serial ", i, "\n")

class MeterMonitor(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        # meter_func.run_meter()
        for i in range(10):
            print ("Serial ",i , "\n")

if __name__ == "__main__":
    Meter = MeterMonitor()
    Serial = SerialMonitor()
    Meter.start()
    time.sleep(2)
    Serial.start()
    while True:
        if not Meter.is_alive():
            Meter = MeterMonitor()
            Meter.start()
        if not Serial.is_alive():
            Serial = SerialMonitor()
            Serial.start()