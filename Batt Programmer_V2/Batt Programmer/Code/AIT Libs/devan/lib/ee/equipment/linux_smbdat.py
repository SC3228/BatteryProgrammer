import logging
from smbus2 import SMBus, i2c_msg
from fcntl import ioctl
import threading
import os
import time
import struct

import sys
import traceback
import inspect

def get_function_name():
    return traceback.extract_stack(None, 2)[0][2]

def get_function_parameters_and_values():
    frame = inspect.currentframe().f_back
    args, _, _, values = inspect.getargvalues(frame)
    return ([(i, values[i]) for i in args])

# IOCTL Codes
I2C_SLAVE = 0x0703    # Use this slave address 
I2C_PEC = 0x0708      # != 0 to use PEC with SMBus
I2C_RETRIES	= 0x0701	# number of times a device address should be polled when not acknowledging
I2C_TIMEOUT	= 0x0702	# set timeout in units of 10 ms

class SmbDatMeta(type):
    """A Parser metaclass that will be used for SmbDat class creation.
    """
    def __instancecheck__(cls, instance):
        return cls.__subclasscheck__(type(instance))

    def __subclasscheck__(cls, subclass):
        return (hasattr(subclass, 'disconnect') and 
                callable(subclass.disconnect) and 
                hasattr(subclass, 'device') and 
                hasattr(subclass, 'write_bytearray') and 
                callable(subclass.write_bytearray) and
                hasattr(subclass, 'read_bytearray') and 
                callable(subclass.read_bytearray) and
                hasattr(subclass, 'write_word') and 
                callable(subclass.write_word) and
                hasattr(subclass, 'read_word') and 
                callable(subclass.read_word) and
                hasattr(subclass, 'write_block') and 
                callable(subclass.write_block) and
                hasattr(subclass, 'read_block') and 
                callable(subclass.read_block) and
                hasattr(subclass, 'send_query') and 
                callable(subclass.send_query) and
                hasattr(subclass, 'raw_write') and 
                callable(subclass.raw_write) and
                hasattr(subclass, 'send_write') and 
                callable(subclass.send_write) and
                hasattr(subclass, 'send_reset') and 
                callable(subclass.send_reset))

class SmbDatInterface(metaclass=SmbDatMeta):
    pass


class LinuxSmbDat(SmbDatInterface):
    CY_SUCCESS = 0
    
    BATTERY_ADDRESS = 0x0B
    
    SMB_BYTE = 0x00
    SMB_WD = 0x01
    SMB_BLK = 0x02
    SMB_LEN_MSK = 0x03
    SMB_WR = 0x04
    SMB_PEC = 0x08
    SMB_RSVD = 0x10  # Prevent opportunity for <ESC> alias

    ERR_OK = 0
    ERR_PEC = -1
    ERR_BLK = -2
    ERR_RD = -3
    
    def __init__(self, device, log=None):
        self._device_name = device
        if log is not None:
            self._log = log.getChild(self.__class__.__name__)
        else:
            log = logging.getLogger(self.__class__.__name__)
            ch = logging.StreamHandler()
            ch.setLevel(logging.INFO)
            formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
            ch.setFormatter(formatter)
            log.addHandler(ch)
            log.setLevel(logging.INFO)
            self._log = log

        self.lock = threading.Lock()
        self._device = None
        
        self.connect()
        
    def connect(self):
        self.__is_connected()
        self._log.debug('Connect')

    def disconnect(self):
        self._device.close()
        self._device = None
        self._log.debug('Disconnect')

    def __is_connected(self):
        if self._device is None:            
            self._device = SMBus(self._device_name)
            self._log.debug('Created SMBus device')
                
        return True

    def send_reset(self):
        self._log.debug('\tReset SMBus device')
        self._device.close()
        self._log.debug('\tDevice closed')
        self._device.open(self._device_name)
        self._log.debug('\tDevice opened')
              
    @property
    def device(self):
        return self._device

    #
    # Internal IOCTL wrappers
    #
    
    @property
    def _fd(self):
        return self._device.fd

    def _set_addr(self, slave_addr):
        ioctl(self._fd, I2C_SLAVE, slave_addr)
        self._log.debug(f'\tSlave address: {hex(slave_addr)}')

    def _set_pec(self, use_pec):
        if use_pec:
          use_pec = 1
          self._log.debug('\tPEC enabled')
        else:
          use_pec = 0
          self._log.debug('\tPEC disabled')
          
        ioctl(self._fd, I2C_PEC, use_pec)

    def _set_timeout(self, timeout_ms):
        ioctl(self._fd, I2C_PEC, timeout_ms)
    
    #    
    # raw bytestream access
    #
    
    def write_bytearray(self, device_address, data, timeout=1000):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        self._set_timeout(timeout)
        self._set_addr(device_address)
        
        msg = i2c_msg.write(device_address, data)
        self._device.i2c_rdwr(msg)
        result = self.CY_SUCCESS  # we don't appear to be able to see if this was NAK'd
              
        self._log.debug(f'{get_function_name()} returns ({result})')
        return result
        
    def read_bytearray(self, device_address, blk_len, timeout=100):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        self._set_timeout(timeout)
        self._set_addr(device_address)       

        msg = i2c_msg.read(device_address, blk_len)
        self._device.i2c_rdwr(msg)
        result = bytes(msg)
        return result

    #
    #  SMBus Access
    #

    def write_word(self, device_address, command_code, data, use_pec=True):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        self._set_pec(use_pec)
        self._set_addr(device_address)
        data = struct.unpack('<H', data)[0]
        return self._device.write_word_data(device_address, command_code, data)
            
    def read_word(self, device_address, command_code, use_pec=True):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        MFG_BLK_RD_NUM_TRIES = 5
    
        response = None
        for i in range(MFG_BLK_RD_NUM_TRIES):
            try:
                self._set_pec(use_pec)
                self._set_addr(device_address)
                response = self._device.read_word_data(device_address, command_code).to_bytes(2, 'little')
            except (ValueError, IOError) as exc:
                self.send_reset()
                time.sleep(0.5)
                if i == (MFG_BLK_RD_NUM_TRIES - 1):
                    raise exc
            else:
                break

        return response

    def write_block(self, device_address, command_code, data, use_pec=True):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        self._set_pec(use_pec)
        self._set_addr(device_address)
        return self._device.write_block_data(device_address, command_code, data)

    def read_block(self, device_address, command_code, use_pec=True, retry_count=5):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        for i in range(retry_count):
            try:
                blk_len = self._read_block(device_address, command_code, 1, False)[0]
                ret_val = self._read_block(device_address, command_code, blk_len, use_pec)
                ret_val = bytearray([len(ret_val)] + ret_val)
                self._log.debug(f'\tReturned ({ret_val!r})')
                return ret_val
            except ValueError:
                self.send_reset()
                
        raise IOError('Failed to read Block')
            
    def _read_block(self, device_address, command_code, blk_len, use_pec=True):
        self._set_pec(use_pec)
        self._set_addr(device_address)
        return self._device.read_block_data(device_address, command_code)
            
    def send_query(self, code, cmd_type):
        with self.lock:
            self.__is_connected()

            if cmd_type & self.SMB_BLK:
                return self.read_block(self.BATTERY_ADDRESS, code, (cmd_type & self.SMB_PEC) != 0)[1:]
            else:
                response = self.read_word(self.BATTERY_ADDRESS, code, (cmd_type & self.SMB_PEC) != 0)               
                return struct.unpack("<H", response)[0]

    def raw_write(self, data):
        with self.lock:
            self.__is_connected()
            return self.write_bytearray(self.BATTERY_ADDRESS, data)
                
    def send_write(self, code, cmd_type, formatted_buffer):
        self._log.debug(f'{get_function_name()}({get_function_parameters_and_values()})')
        with self.lock:
            self.__is_connected()

            ret = None

            if cmd_type & self.SMB_BLK:
                ret = self.write_block(self.BATTERY_ADDRESS,
                                               code,
                                               formatted_buffer,
                                               (cmd_type & self.SMB_PEC) != 0)
            else:
                ret = self.write_word(self.BATTERY_ADDRESS,
                                            code,
                                            formatted_buffer[1:],
                                            (cmd_type & self.SMB_PEC) != 0)            
            self._log.debug(f'\tReturned ({ret!r})')
            if ret == 0 or ret == None:
                return len(formatted_buffer)
            else:               
                return 0
                
if __name__ == '__main__':
    from ee.equipment.smbdat import RolexBattery
    d = LinuxSmbDat(1)
    try:
        b = RolexBattery(None, None, interface=d)
        print(b['ModelNumber'], b['Barcode'], b['Voltage']/1000.0)
    except Exception:
        traceback.print_exception(*sys.exc_info())
