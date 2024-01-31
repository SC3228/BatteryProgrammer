
import os
import smbus
bus = smbus.SMBus(1)

#3 b'\x00'
#write data
#3 b'\x03\x07\x02\x82@\x00\t\xa4'
#write data
#3 b'\x01'

def i2c_write_byte(dev,addr,data):
    for i in range(0,3):
        while True:
            try:
                bus.write_byte_data(dev,addr,data)
            except:
                continue
            break

def i2c_read_byte(dev,addr):
    for i in range(0,3):
        while True:
            try:
                result = bus.read_byte_data(dev,addr)
            except:
                continue
            return result
            break



#wake up the device
i2c_write_byte(0x61,0x00,0x00)
i2c_write_byte(0x61,0x00,0x03)
i2c_write_byte(0x61,0x00,0x07)
i2c_write_byte(0x61,0x00,0x82)
i2c_write_byte(0x61,0x00,0x74)
i2c_write_byte(0x61,0x00,0x00)

i2c_write_byte(0x61,0x00,0xa4)
i2c_write_byte(0x61,0x00,0x00)
print(i2c_read_byte(0x61,0x00) )
print(i2c_read_byte(0x61,0x01) )
print(i2c_read_byte(0x61,0x02) )
print(i2c_read_byte(0x61,0x03) )





#_i2c_dev = os.open("/dev/i2c-1",os.O_RDWR)
#print (_i2c_dev)

