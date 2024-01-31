import platform

from ..base import AtmelCryptoHAL
from ...error import DeviceNotFoundError
from .hid_linux import AtmelCryptoKitHIDLinux, enumerate_kit_devices_linux, connect_kit_device_linux
from .hid_windows import AtmelCryptoKitHIDWindows, enumerate_kit_devices_windows, connect_kit_device_windows

__all__ = ['AtmelCryptoKitHIDLinux', 'AtmelCryptoKitHIDWindows', 'AtmelCryptoKit',
    'ATMEL_CA_VID', 'ATMEL_CA_PID', 'enumerate_kit_devices', 'connect_kit_device'
    ]


class AtmelCryptoKit(AtmelCryptoHAL):
    """
    HAL for communicating with an Atmel Crypto Authentication device via one of
    Atmel's developement kits (e.g. AT88 Microbase USB Dongle). This kits use
    an ASCII protocol and connect to a computer with a Serial or HID
    connection. In either case, the protocol is the same. This class handles
    the protocol.
    """
    def __init__(self, phy):
        self._phy = phy

        self._phy.send(b'board:device(00)\n')

        # Hmmm... On my home box, this blocks indefintely every other time -
        # sending board:device(00) twice works fine though. Works fine on work vm everytime.
        data = self._phy.recv()

        # probably an error: b"C5()" when no device is connected
        if len(data) == 4:
            # TODO: lib/hal/kit_protocol.c
            # This is wrong, but it's fine for now.
            raise DeviceNotFoundError('Atmel Kit failed to find an Atmel Crypto Authentication device.')

        #print(data)
        # assueme the reply is good: "ECC108 TWI 00(C0)"
        # is this protocol even made for machine consumption? ugh.
        p_idx = data.index(b'(') + 1
        address = data[p_idx:p_idx + 2]

        # Are you having fun yet???
        self._phy.send(b's:physical:select(' + address + b')\n')

        # Receive and throw away the response.
        self._phy.recv()

    def wakeup(self):
        self._phy.send(b's:w()\n')

    def recv(self, rx_size):
        data = self._phy.recv()

        # reply looks like this:
        # XX(bunchofhexhere)
        # XX = two (hex?) digit stauts code.
        # bunchofhexhere = the response.
        # TODO: so let's make a stupid assumption for now: the data is perfectly valid
        kitstatus = data[0:2]

        # everything between (), I hope.
        ascii_reply = data[3:-1]

        assert len(ascii_reply) % 2 == 0

        bindata = b''

        idx = 0
        while idx < len(ascii_reply):
            bindata += bytes([int(ascii_reply[idx:idx + 2], 16)])
            idx += 2

        # TODO:
        assert len(bindata) == rx_size
        return bindata

    def send(self, tx_buf):
        """
        Transmit a command to an Atmel Crypto Device via the Kit.
        """
        hex_str = ''.join(['%02X' % byte for byte in tx_buf])

        # sha:talk(...)
        wrapped_buf = b's:t(' + hex_str.encode('ascii') + b')\n'

        self._phy.send(wrapped_buf)

    def idle(self):
        raise NotImplementedError

    def sleep(self):
        raise NotImplementedError

if platform.system() == 'Windows':
    enumerate_kit_devices = enumerate_kit_devices_windows
    connect_kit_device = lambda dev: AtmelCryptoKit(connect_kit_device_windows(dev))
else:
    enumerate_kit_devices = enumerate_kit_devices_linux
    connect_kit_device = lambda dev: AtmelCryptoKit(connect_kit_device_linux(dev))
