"""
``i2c`` structures and constants used to communicate with the Kernel
``i2c-dev`` driver. These are copied from the following files::

    <linux/i2c-dev.h>
    <linux/i2c.h>
    drivers/i2c/i2c-dev.c
"""

from ctypes import Structure, c_uint8, c_uint16, c_uint32, c_char_p, POINTER

"""
/dev/i2c-X ioctl commands. The ioctl's parameter is always an unsigned long,
except for:
    * I2C_FUNCS, takes pointer to an unsigned long
    * I2C_RDWR, takes pointer to struct i2c_rdwr_ioctl_data
    * I2C_SMBUS, takes pointer to struct i2c_smbus_ioctl_data

"""

#: number of times a device address should be polled when not acknowledging
I2C_RETRIES	= 0x0701

#: set timeout in units of 10ms
I2C_TIMEOUT = 0x0702

#: Set the 7-bit slave address to use in subsequent read() or write() calls
I2C_SLAVE = 0x0703

#: Force using this slave address, even if it is already in use by a driver.
I2C_SLAVE_FORCE = 0x0706

#: 0 for 7 bit addresses, != 0 for 10-bit (10-bit is not really supported)
I2C_TENBIT = 0x0704

#: get the adapter functionality mask
I2C_FUNCS = 0x0705

#: Combined R/W transfer. (repeated START condition)
I2C_RDWR = 0x0707

#: use PEC with SMBus
I2C_PEC = 0x0708

#: SMBus transfer
I2C_SMBUS = 0x0720

class i2c_msg(Structure):
    _fields_ = [
        #: 7-bit i2c slave address
        ('addr', c_uint16),

        #: Combination of I2C_M_* constants.
        ('flags', c_uint16),

        ('len', c_uint16),
        ('buf', c_char_p)
    ]

# This is the structure as used in the I2C_RDWR ioctl call.
class i2c_rdwr_ioctl_data(Structure):
    _fields_ = [
        ('msgs', POINTER(i2c_msg)),
        ('nmsgs', c_uint32)
    ]

#: Ten bit slave address
I2C_M_TEN = 0x0010

#: i2c READ 
I2C_M_RD = 0x0001

#: if I2C_FUNC_PROTOCOL_MANGLING
I2C_M_STOP = 0x8000

#: ???
I2C_M_NOSTART = 0x4000

#: if I2C_FUNC_PROTCOL_MANGLING
I2C_M_REV_DIR_ADDR = 0x2000

#: if I2C_FUNC_PROTOCOL_MANGLING
I2C_M_IGNORE_NAK = 0x1000

#: if I2C_FUNC_PROTOCOL_MANGLING
I2C_M_NO_RD_ACK = 0x0800

#: length will be first received byte
I2C_M_RECV_LEN = 0x0400

#: ? 
I2C_RDRW_IOCTL_MAX_MSGS = 42
