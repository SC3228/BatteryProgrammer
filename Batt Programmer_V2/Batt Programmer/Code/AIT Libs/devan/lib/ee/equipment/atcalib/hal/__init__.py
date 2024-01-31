"""
Hardware-abstraction libraries for communicating with an Atmel
crypto-authentication device. These should handle all details lower level than
the actual command bytes sent to the device
"""

from .base import AtmelCryptoHAL

#from .i2c import AtmelCryptoI2CLinux, AtmelCryptoI2CCypress
from .i2c import AtmelCryptoI2CLinux
from .kit import AtmelCryptoKit, AtmelCryptoKitHIDLinux, AtmelCryptoKitHIDWindows, enumerate_kit_devices, connect_kit_device
