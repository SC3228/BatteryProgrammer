import os
_i2c_dev = os.open("/dev/i2c-1",os.O_RDWR)
print (_i2c_dev)

