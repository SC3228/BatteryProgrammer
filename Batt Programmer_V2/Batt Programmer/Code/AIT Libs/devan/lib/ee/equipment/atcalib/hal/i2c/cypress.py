import os
import time

from ..base import AtmelCryptoHAL
from ...atcadev import ATCA_I2C_DEFAULT_ADDRESS
from ee.equipment.cy_smbdat import CyI2cSmbus


class AtmelCryptoI2CCypress(AtmelCryptoHAL):
    """
    ATCA interface via the cypress bridge

    :param i2c_dev: The full path to the ``i2c`` device node, e.g. ``/dev/i2c-2``.
    :param i2c_addr: The 8-bit address of the i2c slave to communicate with.
    """
    def __init__(self, i2c_dev=None, i2c_addr=ATCA_I2C_DEFAULT_ADDRESS):
        if i2c_dev is None:
            self._i2c_dev = CyI2cSmbus()
        else:
            self._i2c_dev = i2c_dev
    
        # Took the 8-bit address for consistency with the datasheet.
        # Linux wants the 7 bit address
        self._i2c_addr = i2c_addr >> 1

    def _write(self, data):
        """
        Issue a WRITE to the i2c slave.

        :param data: Some bytes to write to the i2c device.
        """
        print("debug-1")
        print(self._i2c_addr,data)
        self._i2c_dev.write_bytearray(self._i2c_addr, data)

    def recv(self, rx_size):
        """
        Issue a READ to the i2c slave.
        :return: All bytes read from the slave.
        """
        return self._i2c_dev.read_bytearray(self._i2c_addr, rx_size)

    def sleep(self):
        self._write(b'\x01')
        time.sleep(0.1)
        # self._i2c_dev.write_bytearray(0, b'\x02')
        # time.sleep(0.5)
        
    def wakeup(self):
        """
        Send a wakeup pulse to the device.

        This makes certain assumptions about the i2c bus speed. A wakeup pulse
        requires holding SDA low for 60uS. Sending broadcast address (0x00) at 10 kbps should accomplish this.
        """

        expected = bytes((0x04, 0x11, 0x33, 0x43))
        not_expected = bytes((0x04, 0xFF, 0x01, 0x42))
	
        bus_rate = self._i2c_dev.get_bus_speed()
        # TODO: Why isn't this working with the BQZ78Z101?
        # self._i2c_dev.set_bus_speed(10000)

        time.sleep(0.0004)      
        
        self._i2c_dev.write_bytearray(0, b'\x00', stop_bit=0, timeout=1, nak_bit=1)               
        
        data = bytes()
        
        for i in range(10):
            try:
                time.sleep(.0015)   # Wake Delay
                
                data = self.recv(len(expected))
                if data == expected or data == not_expected:
                    break
            except IOError:
                pass
            
        self._i2c_dev.set_bus_speed(bus_rate)
        
        if data != expected and data != not_expected:
            SLEEP_CMD = b'\x01'
            self.send(SLEEP_CMD)
            raise IOError(F'Invalid status on wake got {data!r} expected {expected!r}')
            #kou7
            #raise IOError('Invalid status on wake got ? expected ?')


    def send(self, tx_buf):
        """
        Send a command and receive the response. This function only handles the
        physical communications. All protocol-level details are elsewhere.

        :param tx_buf: data to transmit.
        """
        # TODO retest this
        # Send the command. 0x03 indicates normal command.
        self._write(b'\x03' + tx_buf)

    def close(self):
        pass
