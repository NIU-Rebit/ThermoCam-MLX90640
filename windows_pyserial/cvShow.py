from irSerial import Serial
import numpy as np
from time import time
import cv2
HIGH = 30
LOW = 20
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
            frame = np.where(frame > 60, 255, 0).astype(np.uint8)
            # frame[frame > 255] = 255
            # frame[frame < 60] = 0
            show = cv2.resize(frame, (320, 240), interpolation=cv2.INTER_NEAREST)
            cv2.imshow('show', show)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    else:
        print ("open serial port error")
finally:
    ser.close()
