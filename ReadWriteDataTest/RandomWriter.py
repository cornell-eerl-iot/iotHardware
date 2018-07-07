import csv
import time
import random
import serial

counter = 100
MAX_BLINKS = 5
INTERVAL = 3 #seconds
file= open('blinks.csv', 'a')
ser = serial.Serial(port='/dev/ttyACM0', baudrate=9600)

while(counter > 0):
    randNum = random.randint(1,MAX_BLINKS)
    file.write('\n,' + str(randNum))
    ser.write(str(randNum))
    print('Random number written: ' + str(randNum))
    counter = counter - 1
    time.sleep(INTERVAL)
file.close()
ser.close()






    
