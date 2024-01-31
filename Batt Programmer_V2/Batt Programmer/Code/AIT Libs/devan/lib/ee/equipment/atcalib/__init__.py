"""
"""

from .atcadev import AtmelCryptoDevice, ATCA_I2C_DEFAULT_ADDRESS
#from .hal.i2c import AtmelCryptoI2CLinux, AtmelCryptoI2CCypress
from .hal.i2c import AtmelCryptoI2CLinux
from .hal.kit import AtmelCryptoKitHIDLinux, AtmelCryptoKit, AtmelCryptoKitHIDWindows,\
    enumerate_kit_devices, connect_kit_device

from .error import DeviceNotFoundError

__all__ = ['atca_i2c_linux', 'atca_kit_hid_linux', 'atca_i2c_cypress', 'atca_kit_hid_windows']


# Arrrrr Here be AtmelCryptoDeviceFactoryMonostateAdapters()
# ... and dragons!
'''
def atca_i2c_cypress(i2c_dev_node=None, i2c_address=ATCA_I2C_DEFAULT_ADDRESS):
    """
    Create an ``AtmelCryptoDevice`` instance using an i2c device on Linux.

    :param i2c_dev_node: The i2c device node under /dev to use [CyI2cSmbus].
    :param i2c_address: The 8-bit i2c address of the ATCA slave.
    """
    dev = AtmelCryptoI2CCypress(i2c_dev_node, i2c_address)

    return AtmelCryptoDevice(dev)
'''

def atca_i2c_linux(i2c_dev_node, i2c_address=ATCA_I2C_DEFAULT_ADDRESS):
    """
    Create an ``AtmelCryptoDevice`` instance using an i2c device on Linux.

    :param i2c_dev_node: The i2c device node under /dev to use.
    :param i2c_address: The 8-bit i2c address of the ATCA slave.
    """
    dev = AtmelCryptoI2CLinux(i2c_dev_node, i2c_address)

    return AtmelCryptoDevice(dev)


def atca_kit_hid(kit_device=None):
    """
    If no ``kit_device`` is given, connect to the first device found on the
    system. If ``kit_device`` is given, behavior is platform-dependent: the
    appropriate ``connect_kit_device_PLATFORM(...)`` function will be called.
    """
    devs = enumerate_kit_devices()

    if len(devs) == 0:
        raise DeviceNotFoundError("No Atmel Crypto Kit devices found.")
    else:
        return AtmelCryptoDevice(connect_kit_device(devs.pop()))
