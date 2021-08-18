from irSerial import Serial
from time import time

ser = Serial('COM16')
try:
    ser.open()
    if ser.isOpen():
        t = time()
        while True:
            frame = ser.getIrFrame()
            print(f'\n{time() - t}, shape={frame.shape}, type={type(frame[0][0])}')
            t = time()
            print('-' * 64)
            for y in range(24):
                for x in range(32):
                    b = frame[y][x] 
                    n = b * 20 / 256 + 20
                    print(int(n), end=' ')
                print()
    else:
        print ("open serial port error")
finally:
    ser.close()
