
import meter_func
import threading
import time
class SerialMonitor(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        meter_func.serial_monitor()

class MeterMonitor(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        meter_func.run_meter()

if __name__ == "__main__":
    Meter = MeterMonitor()
    Serial = SerialMonitor()
    Meter.start()
    time.sleep(3)
    Serial.start()