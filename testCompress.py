
import halfprecisionfloat
import struct
import random

size = 10
comp = halfprecisionfloat.Float16Compressor()
for i in range(size):
    a = random.random()*random.randint(1,100)
    d = int(struct.unpack('I',struct.pack('f',a))[0])
    print d
    b = comp.compress(d)
    b = comp.decompress(b)
    b = float(struct.unpack('f',struct.pack('I',b))[0])
    c = abs(a-b)/a
    print a,b,c

