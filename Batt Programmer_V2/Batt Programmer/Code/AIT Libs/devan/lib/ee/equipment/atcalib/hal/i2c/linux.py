import os
import time

#import smbus


from ..base import AtmelCryptoHAL
from .linux_i2c_dev import *


class AtmelCryptoI2CLinux(AtmelCryptoHAL):
    """
    ATCA interface via the Linux ``i2c-dev`` driver.

    :param i2c_dev: The full path to the ``i2c`` device node, e.g. ``/dev/i2c-2``.
    :param i2c_addr: The 8-bit address of the i2c slave to communicate with.
    """
    def __init__(self, i2c_dev, i2c_addr):
        # Cross-platform: the delayed imports are real
        import fcntl
        self.fcntl = fcntl

        self._i2c_dev = os.open(i2c_dev, os.O_RDWR)
        #self.bus = smbus.SMBus(3)

        # Took the 8-bit address for consistency with the datasheet.
        # Linux wants the 7 bit address
        self._i2c_addr = i2c_addr >> 1
        #lou
        #print("self._i2c_addr =",self._i2c_addr)
        self._set_slave_addr()

    def i2c_read_byte(self,dev,addrs):
        error_count = 0
        while True:
            if error_count > 3:
                return  None            #abort, too many errors
            try:
                value = self.bus.read_byte_data(dev,int(addrs))
            except:
                error_count += 1
                continue
            break
        return value

    def i2c_write_byte(self,dev,addrs,data):
        error_count = 0
        while True:
            if error_count > 3:
                return  None            #abort, too many errors
            try:
                value = self.bus.write_byte_data(dev,int(addrs),data)
            except:
                error_count += 1
                continue
            break
        return True


    def _set_slave_addr(self):
        """
        Instruct the device to use the address given by ``self._i2c_addr`` for
        future read/write operations on the device fd.
        """
        #lou
        #print("ioctl",self._i2c_dev, I2C_SLAVE, self._i2c_addr)
        self.fcntl.ioctl(self._i2c_dev, I2C_SLAVE, self._i2c_addr)

    def _write(self, data):
        """
        Issue a WRITE to the i2c slave.

        :param data: Some bytes to write to the i2c device.
        """
        #lou
        #print('******* write data = ',data)
        error_count = 0
        while True:
            if error_count > 3:
                return  None            #abort, too many errors
            try:
                os.write(self._i2c_dev, data)
            except:
                error_count += 1
                continue
            break


#        os.write(self._i2c_dev, data)

    def recv(self, rx_size):
        """
        Issue a READ to the i2c slave.
        :return: All bytes read from the slave.
        """
        result = os.read(self._i2c_dev, rx_size)
        #lou
        #print("data read",result)
        return result

    def sleep(self):
        self._write(b'\x01')
        time.sleep(0.1)

    def wakeup(self):
        """
        Send a wakeup pulse to the device.

        This makes certain assumptions about the i2c bus speed. A wakeup pulse
        requires holding SDA low for 60uS. At 400KHz, clocking out 8 low bits
        only yields 20uS, but at 100KHz, 80uS. i2c-2 on the BeagleBone black
        runs at 100KHz (note: i2c-0 does not!). The wakeup does not require a
        full i2c transaction with start/address/stop, but this should hurt - we
        end up sending the same sequence as the "reset i/o" command. Ideally we
        would just clock out 0x00.
        """
        try:
            self._write(b'\x00')

        # expected errno 121 - the device will not ACK when it is asleep
        # TODO: This is possibly a problem - will we even clock out the 0x00 if NAK?
        except OSError:
            pass

    def send(self, tx_buf):
        """
        Send a command and receive the response. This function only handles the
        physical communications. All protocol-level details are elsewhere.

        :param tx_buf: data to transmit.
        """
        # TODO retest this
        # Send the command. 0x03 indicates normal command.
        #lou
        #print("tx_buf = ",b'\x03' + tx_buf)

        self._write(b'\x03' + tx_buf)

    def close(self):
        os.close(self._i2c_dev)