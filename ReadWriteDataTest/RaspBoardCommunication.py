import time
import serial

ser = serial.Serial(
    port='COM9',
    baudrate=19200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
    )

#counter="1!This message works asdhfjasdhfkj erhiuewheu^&$#8$^#*&$^*#!"
#ser.write(counter)
#time.sleep(5)

#line = ''
while True:
    #ser.write(counter)
    lineNew = ser.readline()
    #if(line != lineNew):
    print(lineNew)
        
    
ser.close()