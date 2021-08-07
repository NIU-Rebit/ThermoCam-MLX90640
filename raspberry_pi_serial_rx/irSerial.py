import serial
import numpy as np
from time import sleep

class Serial:
    def __init__(self, port) -> None:
        self.ser = serial.Serial()
        self.ser.port = port
        #115200,N,8,1
        self.ser.baudrate = 115200
        self.ser.bytesize = serial.EIGHTBITS #number of bits per bytes
        self.ser.parity = serial.PARITY_NONE #set parity check
        self.ser.stopbits = serial.STOPBITS_ONE #number of stop bits
        self.ser.timeout = 0.5          #non-block read 0.5s
        self.ser.writeTimeout = 0.5     #timeout for write 0.5s
        self.ser.xonxoff = False    #disable software flow control
        self.ser.rtscts = False     #disable hardware (RTS/CTS) flow control
        self.ser.dsrdtr = False     #disable hardware (DSR/DTR) flow control
    
    def open(self):
        self.ser.close()

    def open(self):
        try: 
            self.ser.open()
        except Exception as ex:
            print ("open serial port error " + str(ex))
            return False
    
    def isOpen(self):
        return self.ser.isOpen()
    
    def getIrFrame(self):
        try:
            self.ser.flushInput()
            self.ser.flushOutput()
            self.ser.write(b'A')
            response = self.ser.read(768)
            frame = np.zeros((24, 32), dtype=np.uint8)
            for y in range(24):
                for x in range(32):
                    frame[y][x] = response[y*32+x]
            return frame
        except Exception as ex:
            print ("communicating error " + str(ex))
            return False