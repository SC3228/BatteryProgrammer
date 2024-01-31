
class AtmelCryptoHAL:
    """
    Base class for the PHYsical connection to an Atmel Crypto Authentication
    device.
    """

    def send(self, tx_buf):
        """
        Send command bytes to a physical device.
        """
        raise NotImplementedError

    def recv(self, rx_size):
        """
        Receive a command reply from the device.
        """
        raise NotImplementedError

    def wakeup(self):
        """
        Send a wakeup request to the device.
        """
        raise NotImplementedError

    def sleep(self):
        """
        Put the device to sleep.
        """
        raise NotImplementedError

    def idle(self):
        """
        Put the device into idle mode.
        """
        raise NotImplementedError
