class ATCAError(Exception):
    pass

class DeviceNotFoundError(ATCAError):
    pass

class CRCValidationError(ATCAError):
    pass

class ATCACommandError(ATCAError):
    """
    An error code returned by the device.
    """
    code = None
    msg = 'General Failure!'

    def __init__(self, msg=None, code=None):
        if code is not None:
            self.code = code

        if msg is not None:
            self.msg = msg

        if self.code:
            exc_msg = 'Device responded [0x%02X]: %s' % (self.code, self.msg)
        else:
            exc_msg = self.msg

        super(ATCACommandError, self).__init__(exc_msg)

class WatchDogExpire(ATCACommandError):
    """
    Raised when the ATCA device responds with a "watch dog about to expire"
    (0xEE) error.
    """
    msg = "The watchdog on the device is about to expire."

class CommError(ATCACommandError):
    """
    The command was not properly received by the device and should be
    retransmitted.
    """
    msg = "The command was not properly received by the device. Try again."

class CommandExecutionError(ATCACommandError):
    """
    Command was received by the device, but cannot be executed in its current
    state.
    """
    msg = "The command failed to execute. The device state or parameters were wrong."

class ECCFault(ATCACommandError):
    """
    A computation error occurred during ECC computation. Retrying the command
    may succeed.
    """
    msg = "An error occurred during ECC computation."

class CommandParseError(ATCACommandError):
    """
    Something about the command sent was invalid - sending as is will fail.
    """
    msg = "The command failed to parse."

class MiscompareError(ATCACommandError):
    """
    Command was properly received, but the input client response did not match
    the expected value.
    """
    msg = "The input client response did not match the expected value."

class WakeupError(ATCACommandError):
    """
    Raised when the device sends an unexpected wakeup status.
    """
    msg = "The device just got out of bed."
