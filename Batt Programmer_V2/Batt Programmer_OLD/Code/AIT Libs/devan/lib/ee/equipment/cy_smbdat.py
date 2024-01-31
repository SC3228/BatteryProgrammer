import os

package_directory = os.path.dirname(os.path.abspath(__file__))

from ee.equipment.smbdat import Smbdat

from cy7c65211 import CyUSBSerial
from cy7c65211 import CyI2C
from cy7c65211 import CyGPIO

from ee.utils.crc8 import CRC8

from cffi import FFI

dll = os.path.join(package_directory, '..', '..', 'cy7c65211', 'cyusbserial.dll')

_CY_RETURN_STATUS = [
    'CY_SUCCESS',
    'CY_ERROR_ACCESS_DENIED',
    'CY_ERROR_DRIVER_INIT_FAILED',
    'CY_ERROR_DEVICE_INFO_FETCH_FAILED',
    'CY_ERROR_DRIVER_OPEN_FAILED',
    'CY_ERROR_INVALID_PARAMETER',
    'CY_ERROR_REQUEST_FAILED',
    'CY_ERROR_DOWNLOAD_FAILED',
    'CY_ERROR_FIRMWARE_INVALID_SIGNATURE',
    'CY_ERROR_INVALID_FIRMWARE',
    'CY_ERROR_DEVICE_NOT_FOUND',
    'CY_ERROR_IO_TIMEOUT',
    'CY_ERROR_PIPE_HALTED',
    'CY_ERROR_BUFFER_OVERFLOW',
    'CY_ERROR_INVALID_HANDLE',
    'CY_ERROR_ALLOCATION_FAILED',
    'CY_ERROR_I2C_DEVICE_BUSY',
    'CY_ERROR_I2C_NAK_ERROR',
    'CY_ERROR_I2C_ARBITRATION_ERROR',
    'CY_ERROR_I2C_BUS_ERROR',
    'CY_ERROR_I2C_BUS_BUSY',
    'CY_ERROR_I2C_STOP_BIT_SET',
    'CY_ERROR_STATUS_MONITOR_EXIST'
]


class CY_ERROR_I2C_NAK_ERROR(IOError):
    pass


def zebra_finder(serial_str=b'0007', mfg_str=b'Zebra Technologies', product_str=b'Battery Test Fixture Bd Rev A'):
    def finder(info):
        ffi = FFI()

        serial = ffi.string(info.serialNum)
        mfg = ffi.string(info.manufacturerName)
        prd = ffi.string(info.productName)

        return (serial == serial_str) and (mfg == mfg_str) and (prd == product_str)

    return finder

class CyI2cSmbus(object):
    lib = None

    def __init__(self, device=None, finder=None, serial=None):

        if device is None:
            CyI2cSmbus.load_cypress_lib(dll)

            if serial:
                self.dev = next(CyI2cSmbus.lib.find(finder=zebra_finder(serial_str=serial)))
            elif finder:
                self.dev = next(CyI2cSmbus.lib.find(finder=finder))
            else:
                self.dev = next(CyI2cSmbus.lib.find(vid=0x04B4, pid=0x0007))
            self.dev.raise_on_error = False

            self.i2c_dev = CyI2C(self.dev)
            self.i2c_dev.debug = 0
            self.gpio = CyGPIO(self.dev)
        else:
            self.i2c_dev = device

    def get_bus_speed(self):
        return self.i2c_dev.get_config()['frequency']
            
    def set_bus_speed(self, rate):
        cfg = self.i2c_dev.get_config()
        cfg['frequency'] = rate
        self.i2c_dev.set_config(cfg)
            
    @classmethod
    def load_cypress_lib(cls, dll_path=dll):
        if cls.lib is None:
            cls.lib = CyUSBSerial(lib=dll_path)

    def write_bytearray(self, device_address, data, stop_bit=1, timeout=1000, nak_bit=0):
        if type(data) is not bytearray and type(data) is not bytes:
            raise TypeError("Data format is unexpected")

        cfg = self.i2c_dev.prepare(device_address, isStopBit=stop_bit, isNakBit=nak_bit)
        rc = self.i2c_dev.write(cfg, data, timeout=timeout)

        return rc

    def read_bytearray(self, device_address, blk_len, nack=1):
        buffer = bytearray([0x0] * blk_len)

        cfg = self.i2c_dev.prepare(device_address, isStopBit=1, isNakBit=nack)    #Initialize the cfg variable like before.  This time we set the NakBit=0 so that a NAK occurs at the end of this line indicating that the block_read has ended
        rc = self.i2c_dev.read(cfg, buffer, timeout=1000)

        return rc

    CY_SUCCESS = 0
    SMBUS_WD_LEN = 2

    def read_word(self, device_address, command_code, use_pec=True):
        MFG_BLK_RD_NUM_TRIES = 5
    
        response = None
        for i in range(MFG_BLK_RD_NUM_TRIES):
            try:
                response = self._read_word(device_address, command_code, use_pec)
            except (ValueError, IOError) as exc:
                self.send_reset()
                time.sleep(0.5)
                if i == (MFG_BLK_RD_NUM_TRIES - 1):
                    raise exc
            else:
                break

        return response
    
    def _read_word(self, device_address, command_code, use_pec=True):
        pec = 0
        string_data = bytearray([(device_address << 1), ])  #bit 7-bit address to match transmitted field
        string_data.append(command_code)
        rc = self.write_bytearray(device_address, string_data[1:],stop_bit=0)

        if rc != self.CY_SUCCESS:
            self.i2c_dev.reset(1)
            print(F'IOTIMEOUT ({device_address:x})')
            raise IOError(_CY_RETURN_STATUS[rc])

        blk_len = self.SMBUS_WD_LEN
        if use_pec:
            blk_len += 1
        response = self.read_bytearray(device_address, blk_len)

        if use_pec:
            for cb in string_data:
                pec = CRC8(pec, cb)
            pec = CRC8(pec, (device_address << 1) | 1)
            for cb in response[:-1]:
                pec = CRC8(pec, cb)

            if pec != response[-1]:
                raise ValueError("PEC value mismatch on cmd %02x, expected %r, got %r : %r" % (command_code,
                                                                                               pec,
                                                                                               response[-1],
                                                                                               response))

        if use_pec:
            return response[:-1]
        return response

    def write_word(self, device_address, command_code, data, use_pec=True):
        pec = 0
        string_data = bytearray([(device_address << 1), ])
        string_data.append(command_code)
        string_data.extend(data[:2])

        if use_pec:
            for cb in string_data:
                pec = CRC8(pec, cb)
            string_data.append(pec)

        rc = self.write_bytearray(device_address, string_data[1:])

        if _CY_RETURN_STATUS[rc] == 'CY_ERROR_I2C_NAK_ERROR':
            raise CY_ERROR_I2C_NAK_ERROR()

        if rc != self.CY_SUCCESS:
            self.i2c_dev.reset(1)
            raise IOError(_CY_RETURN_STATUS[rc])

        return rc

    def send_reset(self, reset_type=1):
        assert self.i2c_dev is not None
        self.i2c_dev.reset(reset_type)

    def _read_block(self, device_address, command_code, blk_len, use_pec=True):
        pec = 0
        string_data = bytearray([(device_address << 1), ])  #bit 7-bit address to match transmitted field
        string_data.append(command_code)
        rc = self.write_bytearray(device_address, string_data[1:],stop_bit=0)

        if rc != self.CY_SUCCESS:
            #print(_CY_RETURN_STATUS[rc])
            self.i2c_dev.reset(1)
            raise IOError(_CY_RETURN_STATUS[rc])

        blk_len += 1        # for length byte
        if use_pec:
            blk_len += 1    # for PEC byte
        print(F'Reading {blk_len} bytes')
        response = self.read_bytearray(device_address, blk_len, nack=1)
        print(F'Recieved {len(response)} bytes as {response.hex()}')
        
        if use_pec:
            for cb in string_data:
                pec = CRC8(pec, cb)
            pec = CRC8(pec, (device_address << 1) | 1)
            for cb in response[:-1]:
                pec = CRC8(pec, cb)

            if pec != response[-1]:
                # pec = 0
                # for cb in string_data:
                    # pec = CRC8(pec, cb)
                    # print(F'{cb:02x} -> {pec:02x}')
                # pec = CRC8(pec, (device_address << 1) | 1)
                # print(F'{(device_address << 1) | 1:02x} -> {pec:02x}')
                # for cb in response[:-1]:
                    # pec = CRC8(pec, cb)
                    # print(F'{cb:02x} -> {pec:02x}')
                raise ValueError("PEC value mismatch on blk cmd 0x%02x, expected %#04x, got : %#04x : %s" % (command_code,
                                                                                                             pec,
                                                                                                             response[-1],
                                                                                                             response.hex()))

        if use_pec:
            return response[:-1]
        return response

    def read_block(self, device_address, command_code, use_pec=True, retry_count=5):
        for i in range(retry_count):
            try:
                print(F'Getting block size for command {command_code:02x} on device {device_address:02x}...')
                blk_len = self._read_block(device_address, command_code, 1, False)[0]
                print(F'Block len {blk_len}.')
                return self._read_block(device_address, command_code, blk_len, use_pec)
            except ValueError:
                self.send_reset()
                
        raise IOError('Failed to read Block')

    def write_block(self, device_address, command_code, data, use_pec=True):
        string_data = bytearray([device_address<<1, ])                             #Bit shift of the address of the device is used for the first value of the string being written in block_write
        string_data.append(command_code)                                    #Add the command to the block_write string that is written
        string_data.append(len(data))                                 #Calculte the length in bytes of the data being written and add to to the block_write string being written
        string_data.extend(data)                                      #Add the entire contents of 'write_data' to the block_write string
        pec = 0                                                             #Initialize the PEC variable

        if use_pec:
            for cb in string_data:
                pec = CRC8(pec, cb)
            string_data.append(pec)

        rc = self.write_bytearray(device_address, string_data[1:])

        if rc != self.CY_SUCCESS:
            self.i2c_dev.reset(1)
            raise IOError(_CY_RETURN_STATUS[rc])

        return rc

import time, struct


class CySmbDat(Smbdat):
    CY_SUCCESS = 0

    def disconnect(self):
        self._device = None

    def __is_connected(self):
        if self._device is None:
            if self._device_name:
                self._device = self._device_name
            else:
                self._log.debug('Creating Connection')
                self._device_name = self._device = CyI2cSmbus()

        self.i2c_dev = self._device.i2c_dev
                
        return True

    def send_reset(self):
        with self.lock:
            self.__is_connected()
            self._device.i2c_dev.reset(1)
            time.sleep(0.1)

    def write_word(self, *args, **kwargs):
        return self._device.write_word(*args, **kwargs)
            
    def read_word(self, *args, **kwargs):
        return self._device.read_word(*args, **kwargs)
            
    def write_bytearray(self, *args, **kwargs):
        return self._device.write_bytearray(*args, **kwargs)
            
    @property
    def device(self):
        return self._device
            
    def send_query(self, code, cmd_type):
        with self.lock:
            self.__is_connected()

            if cmd_type & self.SMB_BLK:
                response = self._device.read_block(self.BATTERY_ADDRESS, code, (cmd_type & self.SMB_PEC) != 0)[1:]
                return response
            else:
                response = self._device.read_word(self.BATTERY_ADDRESS, code, (cmd_type & self.SMB_PEC) != 0)
                return struct.unpack("<H", response)[0]

    def raw_write(self, data):
        with self.lock:
            self.__is_connected()
            return self._device.write_bytearray(self.BATTERY_ADDRESS, data)
                
    def send_write(self, code, cmd_type, formatted_buffer):
        with self.lock:
            self.__is_connected()

            ret = None

            if cmd_type & self.SMB_BLK:
                ret = self._device.write_block(self.BATTERY_ADDRESS,
                                               code,
                                               formatted_buffer,
                                               (cmd_type & self.SMB_PEC) != 0)
            else:
                ret = self._device.write_word(self.BATTERY_ADDRESS,
                                            code,
                                            formatted_buffer[1:],
                                            (cmd_type & self.SMB_PEC) != 0)
            if ret == 0:
                return len(formatted_buffer)
            else:
                return 0


import argparse
from zebra.util import logger
from ee.equipment.smbdat import SmartBattery, PanasonicSmartBattery, ICCSmartBattery, RolexBattery

def run_example():
    import traceback

    s_parser = Smbdat.create_parser()
    parser = argparse.ArgumentParser(
        description='A smart battery data test',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        parents=[s_parser]
    )
    parser.add_argument("--log_level",
                        type=int,
                        default=logger.INFO,
                        )
    args = parser.parse_args()

    # parser, args = DischargeTest.parse_cmdline()
    log = logger.get_logger("test", level=args.log_level)

    lib = CyUSBSerial(lib = dll)
    dev = lib.find(vid=0x04B4, pid=0x0007).next()
    dev.raise_on_error = False

    i2c_dev = CyI2C(dev)
    i2c_dev.debug = False
    smbus_interface = CyI2cSmbus(device=i2c_dev)
    battery_interface = CySmbDat(device=smbus_interface, log=log)

    gpio_dev = CyGPIO(dev)

    GPIO_LOAD = 18
    GPIO_SUPPLY = 17

    def dut_power_config(supply, load):
        if gpio_dev:
            gpio_dev.set(GPIO_SUPPLY, supply & 1)
            gpio_dev.set(GPIO_LOAD, load & 1)
            time.sleep(1)

    battery = None
    dut_power_config(True, False)

    try:
        battery = SmartBattery(args.batt_com, log, interface=battery_interface)
        if(battery['ManufacturerName'] in ('Panasonic', 'SANYO') ):
            battery = PanasonicSmartBattery(args.batt_com, log, interface=battery_interface)
            print("Panasonic battery detected")
        elif(battery['ManufacturerName'] in ('ICC', 'ICCN')):
            battery = ICCSmartBattery(args.batt_com, log, interface=battery_interface)
            print("ICC battery detected")
        elif(battery['ManufacturerName'] in ('Zebra Technologies',)):
            battery = RolexBattery(args.batt_com, log, interface=battery_interface)
            print("Rolex battery detected")
    except Smbdat.CommandError:
        dut_power_config(False, False)
        pass
    finally:
        print("Unknown battery")
        if battery is None:
            battery = SmartBattery(args.batt_com, log, interface=battery_interface)

    for key in battery:
        print(("%25s(%#04x) = " % (key, SmartBattery.SMBDAT_COMMANDS[key]))),
        try:
            response = battery[key]
        except ValueError as e:
            battery.send_reset()
            print('Send Command exception occurred, value:', repr(e))
            # traceback.print_exc()
            # print("ERR(%i)" % (e.value,))
        else:
            try:
                if (type(response) is str) and not (key in SmartBattery.SMBDAT_AS_HEX):
                    print("%s" % (response,))
                elif type(response) is int:
                    if key in SmartBattery.SMBDAT_AS_HEX:
                        print("%#04x" % (response,))
                    else:
                        print("%i" % (response,))
                else:
                    print("%s" % (":".join("{:02x}".format(ord(c)) for c in response),))

                if key == 'ManufactureDate':
                    print('%s%d/%d/%d' % (' ' * 35, (response & 0x1F0) >> 5, response & 0x1F, ((response & 0xFE00) >> 9) + 1980))
                elif key == 'DateFirstUsed':
                    date = struct.unpack('<H', response[11:11+2])[0]  # ord(response[11]) | (ord(response[12]) << 8)
                    print('%s%d/%d/%d' % (' ' * 35, (date & 0x1F0) >> 5, date & 0x1F, ((date & 0xFE00) >> 9) + 1980))
                elif key == "LifetimeData" and isinstance(battery, PanasonicSmartBattery):
                    # Byte 1,2: Max. cell temperature [0.1K]
                    # Byte 3,4: Min. celll temperature [0.1K]
                    # Byte 5,6: Max. cell voltage [mV]
                    # Byte 7,8: Min. cell voltage [mV]
                    # Byte 9,10: Max. pack voltage [mV]
                    # Byte 11,12: Wakeup count [Times]
                    # Byte 13,14: Max. charge current [mA]
                    # Byte 15,16: Max. discharge current [mA]
                    # Byte 17,18: Max. charge power [100mW]
                    # Byte 19,20: Max. discharge power [100mW]
                    # Byte 21,22: Max. average discharge current [mA]
                    # Byte 23,24: Max. average discharge power [100mW]
                    # Byte 25,26: Temp < 10degC [hours]
                    # Byte 27,28: 10 =< Temp < 45degC [hours]
                    # Byte 29,30: 45 =< Temp < 60degC [hours]
                    # Byte 31,32: Temp >= 60degC [hours]
                    fields = ['Byte 1,2: Max. cell temperature [0.1K]',
                              'Byte 3,4: Min. celll temperature [0.1K]',
                              'Byte 5,6: Max. cell voltage [mV]',
                              'Byte 7,8: Min. cell voltage [mV]',
                              'Byte 9,10: Max. pack voltage [mV]',
                              'Byte 11,12: Wakeup count [Times]',
                              'Byte 13,14: Max. charge current [mA]',
                              'Byte 15,16: Max. discharge current [mA]',
                              'Byte 17,18: Max. charge power [100mW]',
                              'Byte 19,20: Max. discharge power [100mW]',
                              'Byte 21,22: Max. average discharge current [mA]',
                              'Byte 23,24: Max. average discharge power [100mW]',
                              'Byte 25,26: Temp < 10degC [hours]',
                              'Byte 27,28: 10 =< Temp < 45degC [hours]',
                              'Byte 29,30: 45 =< Temp < 60degC [hours]',
                              'Byte 31,32: Temp >= 60degC [hours]']
                    data = struct.unpack(">HHhhhHhhhhhhHHHH", response)
                    for field, val in zip(fields,data):
                        print("%s%s : %d(%x)" % (' ' * 35, field, val, val))
                elif key == "ManufacturerData" and isinstance(battery, PanasonicSmartBattery):
                    # Byte 1 : Software model Code
                    # Byte 2 : Software version Number
                    # Byte 3 : EEPROM/Flash Data type code
                    # Byte 4 : EEPROM/Flash data version
                    # Byte 5, 6:Cell voltage of V4 (Not used)
                    # Byte 7, 8:Cell voltage of V3 (Not used)
                    # Byte 9,10:Cell voltage of V2
                    # Byte 11,12:Cell voltage of V1
                    fields = ['Byte 1 : Software model Code',
                              'Byte 2 : Software version Number',
                              'Byte 3 : EEPROM/Flash Data type code',
                              'Byte 4 : EEPROM/Flash data version',
                              'Byte 5, 6:Cell voltage of V4 (Not used)',
                              'Byte 7, 8:Cell voltage of V3 (Not used)',
                              'Byte 9,10:Cell voltage of V2',
                              'Byte 11,12:Cell voltage of V1']
                    data = struct.unpack("<BBBBHHHH", response)
                    for field, val in zip(fields,data):
                        print('%s%s : %d' % (' ' * 35, field, val))
                elif key == "ManufacturerAccess" and isinstance(battery, PanasonicSmartBattery):
                    fields = [('Pre-Charge Timeout', 0x0001),
                              ('Cell Short Error (charging)', 0x0002),
                              ('Cell Imbalance Error 1', 0x0004),
                              ('Discharge FET status', 0x4000),
                              ('Charge FET status', 0x8000)
                              ]

                    for field, msk in fields:
                        print("%s%s : %d" % (' ' * 35, field, response & msk != 0))

                    status =   ['System failure (see 2.4.17 -> 2.4.19)(ERR_AFE, ERR_MEMORY, ERR_CHECKSUM)',
                                'Reserved',
                                'Pre-Charge Timeout (see 2.4.7)',
                                'Charge time out (see 2.4.6)',
                                'Charge Over current detected by H/W (see 2.4.5.1)',
                                'Discharge over current detected by H/W (see 2.4.5.1)',
                                'Charge over current detected by S/W(Pre charge or Normal charge) (see 2.4.5.2)',
                                'Discharge over current detected by S/W (see 2.4.5.2)',
                                'Over Charge Capacity (see 2.4.8)',
                                'Over charge detected by S/W(1st,4.23V) (see 2.4.1)',
                                'Over discharge (see 2.4.4)',
                                'Over temperature for discharge (Cell Temperature Protection 2) (see 2.4.11), OR FET Temperature Protection 1 (see 2.4.13)',
                                'Over / Under temperature for charge (Cell Temperature Protection 1) (see 2.4.10)',
                                'Fully charged (see 2.1.3)',
                                'Reserved',
                                'Normal',
                                ][::-1]

                    print("%s%s : %s" % (' ' * 35, 'Status 1', status[(response & 0x0F00) >> 8]))

                    status =   ['Reserved',
                                'Reserved',
                                'Reserved',
                                'Reserved',
                                'Reserved',
                                'Reserved',
                                'Reserved',
                                'Tap error (see 2.4.20)',
                                'FET temp Protection 2 (see 2.4.14)',
                                'Cell temp Protection 3 (see 2.4.12)',
                                'Cell Imbalance Error 2 (see 2.4.16)',
                                'Reserved',
                                'FET Failure (see 2.4.9)',
                                'Over charge detect by SW (2nd,4.32 V) (see 2.4.2)',
                                'Over charge detect by HW (3rd,4.40V) (see 2.4.3)',
                                'Normal',
                                ][::-1]

                    print("%s%s : %s" % (' ' * 35, 'Status 2', status[(response & 0x00F0) >> 4]))
            except struct.error as e:
                import sys
                sys.stdout.flush()
                log.exception("<failed to unpack>")
                sys.stderr.flush()
                time.sleep(.1)
    dut_power_config(False, False)

if __name__ == "__main__":
    run_example()
