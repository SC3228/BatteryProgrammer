import time
import struct
from zebra.io import unity
from zebra.util import logger
import argparse
from ctypes import *
from zstruct import Struct, Type, Format, ArrayElement
from collections import OrderedDict, MutableMapping


class Smbdat(object):

    def __init__(self, device, log=None):
        self._device_name = device
        self._log = log
        if log is not None:
            self._log = log.getChild(self.__class__.__name__)
            self._log.setLevel(logger.INFO)

        self.lock = threading.Lock()
        self._device = None
        
    @staticmethod
    def create_parser(): 
        parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
            add_help=False
        )
    
        group = parser.add_argument_group("Instruments")
        group.add_argument("--batt_com",
                           type=str,
                           default="COM30",
                           help="COM port for SMBUS device")                               
        return parser

    def __del__(self):
        self.disconnect()

    def disconnect(self):
        if self._device is not None:
            self._device.disconnect()
            self._device = None
            self._log.debug('Closed Connection')

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
        
    class CommandError(Exception):
        def __init__(self, value):
            self.value = value
            
        def __str__(self):
            return repr(self.value)

    def __is_connected(self):
        # TODO: This is terrible, need to rewrite command parser (Arduino) and connection routine
        if self._device is None:
            self._log.debug('Creating Connection')
            self._device = unity.SerialConnection(self._device_name, 115200, log=self._log)
            self._device.send("\x1B\x1B\x1B\x1BE")
            if "Hello\r\n" != self._device.collect_sincelast(0.2, 2.0):
                print("uh oh")
        # else:
            # self._device.disconnect()
            # time.sleep(0.1)
            # self._device.connect()
            # self._device.send("EEEE")
            # print(repr(self._device.collect_sincelast(0.2, 2.0)))

    def send_reset(self):
        with self.lock:
            self.__is_connected()

            self._device.send("\x1B\x1B\x1B\x1BR")
            time.sleep(0.1)

    START_CODE = 0x1B
    BATTERY_ADDRESS = 0x0B
    SEND_COMMAND = ord('S')

    def send_query(self, code, cmd_type):
        import binascii

        xmit_string = struct.pack('<BBBBB',
                                  self.START_CODE,
                                  self.SEND_COMMAND,
                                  self.BATTERY_ADDRESS,
                                  code,
                                  cmd_type)

        with self.lock:
            self.__is_connected()

            self._device.send(xmit_string)
            if cmd_type & self.SMB_BLK:
                response = self._device.collect_len(1, timeout=1.0)
                status = struct.unpack("<b", response)[0]
                # print("stage1(%s)" % (binascii.hexlify(response),))
                # print(repr(response))
                if status == 0:
                    response = self._device.collect_len(1, timeout=1.0)
                    length = struct.unpack("<B", response)[0]
                    response = self._device.collect_len(length, timeout=1.0)
                    # print("stage2(%s)" % (binascii.hexlify(response),))
                    return response

            else:
                response = self._device.collect_len(3, timeout=1.0)
                # print(repr(response))
                # print(len(response))
                if len(response) == 1:
                    status = struct.unpack("<b", response)[0]
                elif len(response) == 3:
                    status = struct.unpack("<b2x", response)[0]
                else:
                    print("ERROR(%s)" % (binascii.hexlify(response),))
                    raise self.CommandError(0xDEAD)

            if status != 0:
                raise self.CommandError(status)

        return struct.unpack("<xH", response)[0]

    def send_write(self, code, cmd_type, formatted_buffer):

        xmit_string = struct.pack('<BBBBB',
                                  self.START_CODE,
                                  self.SEND_COMMAND,
                                  self.BATTERY_ADDRESS,
                                  code,
                                  cmd_type)

        xmit_string += formatted_buffer

        with self.lock:
            self.__is_connected()

            self._device.send(xmit_string)
            response = self._device.collect_len(1, timeout=1.0)
            status = struct.unpack("<b", response)[0]

        return status


class SmartBattery(MutableMapping):
    def __init__(self, device_name, log, interface=None):
        self.log = logger.get_library_logger('SmartBattery', parent=log)
        if interface is None:
            self.interface = Smbdat(device_name, log)
        else:
            self.interface = interface

    def __getitem__(self, key):
        if type(key) is not str:
            raise TypeError

        if key in self.SMBDAT_COMMANDS:
            # print('Reading %s' % (key,))
            return self.read(key)
        else:
            raise IndexError('Command not found', key)

    def __setitem__(self, key, value):
        if type(key) is not str:
            raise TypeError

        if key in self.SMBDAT_COMMANDS:
            return self.write(key, value)
        else:
            raise IndexError('Command not found', key)

    def __delitem__(self, key):
        pass

    def __iter__(self):
        return self.SMBDAT_COMMANDS.__iter__()

    def __len__(self):
        return len(self.SMBDAT_COMMANDS)

    SMBDAT_COMMANDS = OrderedDict([
        ("ManufacturerAccess", 0x00),
        ("BatteryMode", 0x03),
        ("AtRate", 0x04),
        ("AtRateTimeToFull", 0x05),
        ("AtRateTimeToEmpty", 0x06),
        ("Temperature", 0x08),
        ("Voltage", 0x09),
        ("Current", 0x0A),
        ("AverageCurrent", 0x0B),
        ("MaxError", 0x0C),
        ("RelativeStateOfCharge", 0x0D),
        ("AbsoluteStateOfCharge", 0x0E),
        ("RemainingCapacity", 0x0F),
        ("FullChargeCapacity", 0x10),
        ("AverageTimeToEmpty", 0x12),
        ("AverageTimeToFull", 0x13),
        ("BatteryStatus", 0x16),
        ("CycleCount", 0x17),
        ("DesignCapacity", 0x18),
        ("DesignVoltage", 0x19),
        ("SpecificationInfo", 0x1a),
        ("ManufactureDate", 0x1b),
        ("SerialNumber", 0x1c),
        ("ManufacturerName", 0x20),
        ("DeviceName", 0x21),
        ("DeviceChemistry", 0x22),
        ("ManufacturerData", 0x23),
        ("Authentication", 0x2F),
        ("DateFirstUsed", 0x38)])
        
    SMB_BYTE = Smbdat.SMB_BYTE
    SMB_WD = Smbdat.SMB_WD
    SMB_BLK = Smbdat.SMB_BLK
    SMB_PEC = Smbdat.SMB_PEC       
        
    SMBDAT_COMMAND_TYPE = {
        "ManufacturerAccess": SMB_WD,
        "RemainingCapacityAlarm": SMB_WD | SMB_PEC,
        "RemainingTimeAlarm": SMB_WD | SMB_PEC,
        "BatteryMode": SMB_WD | SMB_PEC,
        "AtRate": SMB_WD | SMB_PEC,
        "AtRateTimeToFull": SMB_WD | SMB_PEC,
        "AtRateTimeToEmpty": SMB_WD | SMB_PEC,
        "AtRateOK": SMB_WD | SMB_PEC,
        "Temperature": SMB_WD | SMB_PEC,
        "Voltage": SMB_WD | SMB_PEC,
        "Current": SMB_WD | SMB_PEC,
        "AverageCurrent": SMB_WD | SMB_PEC,
        "MaxError": SMB_WD | SMB_PEC,
        "RelativeStateOfCharge": SMB_WD | SMB_PEC,
        "AbsoluteStateOfCharge": SMB_WD | SMB_PEC,
        "RemainingCapacity": SMB_WD | SMB_PEC,
        "FullChargeCapacity": SMB_WD | SMB_PEC,
        "RunTimeToEmpty": SMB_WD | SMB_PEC,
        "AverageTimeToEmpty": SMB_WD | SMB_PEC,
        "AverageTimeToFull": SMB_WD | SMB_PEC,
        "BatteryStatus": SMB_WD | SMB_PEC,
        "CycleCount": SMB_WD | SMB_PEC,
        "DesignCapacity": SMB_WD | SMB_PEC,
        "DesignVoltage": SMB_WD | SMB_PEC,
        "SpecificationInfo": SMB_WD | SMB_PEC,
        "ManufactureDate": SMB_WD | SMB_PEC,
        "SerialNumber": SMB_WD | SMB_PEC,
        "ManufacturerName": SMB_BLK | SMB_PEC,
        "DeviceName": SMB_BLK | SMB_PEC,
        "DeviceChemistry": SMB_BLK | SMB_PEC,
        "ManufacturerData": SMB_BLK | SMB_PEC,
        "CycleCountThreshold": SMB_WD | SMB_PEC,
        "CapacityRatio": SMB_WD | SMB_PEC,
        "StateofChargeThreshold": SMB_WD | SMB_PEC,
        "LEDBlinkRate": SMB_WD | SMB_PEC,
        "OnTime": SMB_WD | SMB_PEC,
        "OffTime": SMB_WD | SMB_PEC,
        "LEdDisable": SMB_WD | SMB_PEC,
        "LifetimeData": SMB_BLK | SMB_PEC,
        "Authentication": SMB_BLK | SMB_PEC,
        "DateFirstUsed": SMB_BLK | SMB_PEC,
        "CellModelNo": SMB_BLK | SMB_PEC,
        "Barcode": SMB_BLK | SMB_PEC,
        "ModelNumber": SMB_BLK | SMB_PEC,
    }
        
    SMBDAT_AS_HEX = (
        "BatteryMode",
        "DateFirstUsed",
        "ManufacturerAccess",
        "BatteryStatus",
        "ManufactureDate",
        "SerialNumber",
        "ManufacturerData",
        "LifetimeData",
        "Authentication"
    )

    SMBDAT_IS_SIGNED = (
        "AtRate",
        "AverageCurrent",
        "Current",
        "AverageCurrent"
    )

    def _is_signed(self, command):
        return command in self.SMBDAT_IS_SIGNED

    def __check_sign(self, value, command):
        if self._is_signed(command):
            c_value = c_ushort(value)
            pc_value = pointer(c_value)
            c_signed = cast(pc_value, POINTER(c_short))
            return c_signed.contents.value
        else:
            return value

    def read(self, cmd):
        if cmd in self.SMBDAT_COMMANDS:
            retval = self.interface.send_query(self.SMBDAT_COMMANDS[cmd], self.SMBDAT_COMMAND_TYPE[cmd])
            if type(retval) is int:
                retval = self.__check_sign(retval, cmd)
            else:
                retval = retval
            return retval

    def write(self, cmd, buffer):
        if cmd in self.SMBDAT_COMMANDS:
            is_signed = self._is_signed(cmd)
            cmd_type = self.SMBDAT_COMMAND_TYPE[cmd]

            if cmd_type & self.SMB_BLK:
                formatted_buffer = struct.pack('<' + str(len(buffer)) + 's', buffer)
            else:
                if cmd_type & self.SMB_WD:
                    length = 2
                    format_char = 'H'
                else:
                    length = 1
                    format_char = 'B'

                if is_signed:
                    format_char = format_char.lower()
                else:
                    format_char = format_char.upper()

                formatted_buffer = struct.pack('<B' + format_char, length, buffer)

            retval = self.interface.send_write(self.SMBDAT_COMMANDS[cmd],
                                     self.SMBDAT_COMMAND_TYPE[cmd] | Smbdat.SMB_WR,
                                     formatted_buffer)

            if retval != len(formatted_buffer):
                raise IOError('SMBDAT transmission failed to complete (sent %d of %d bytes)',
                              retval,
                              len(formatted_buffer))

            return retval

    def send_reset(self):
        self.interface.send_reset()


from ee.equipment.agilent34970A import Agilent34970A


class DAQBattery(SmartBattery):
    SMBDAT_COMMANDS = OrderedDict([
        ("Temperature", (101,)),
        ("Voltage", (102,)),
    ])

    SMB_WD = Smbdat.SMB_WD

    SMBDAT_COMMAND_TYPE = {
        "Temperature": SMB_WD,
        "Voltage": SMB_WD,
    }

    def __del__(self):
        pass

    def __init__(self, device_name, log=None):

        daq = Agilent34970A(device_name, log)

        super(DAQBattery, self).__init__(device_name, log, interface=daq)

        self.interface.set_conf_temp(self.SMBDAT_COMMANDS['Temperature'], 'THERmistor', 10000)
        self.interface.set_conf_vdc(self.SMBDAT_COMMANDS['Voltage'])
        self.interface.set_trig_src(Agilent34970A.TrigSrc.IMMediate)

    def read(self, cmd):
        if cmd in self.SMBDAT_COMMANDS:
            self.interface.set_scan(self.SMBDAT_COMMANDS[cmd])
            return self.interface.read()[0]
        else:
            return 0

    def write(self, cmd, buffer):
        raise NotImplementedError

    def send_reset(self):
        pass


class PanasonicSmartBattery(SmartBattery):
    def __init__(self, device_name, log, interface=None):
        super(PanasonicSmartBattery, self).__init__(device_name, log, interface)

        self.supports_led = False
        try:
            self.interface.send_query(0x2B, SmartBattery.SMB_WD | SmartBattery.SMB_PEC)
            self.supports_led = True
        except Smbdat.CommandError:
            pass

        if self.supports_led:
            self.SMBDAT_COMMANDS.update( OrderedDict([
                ("CycleCountThreshold", 0x28),
                ("CapacityRatio", 0x29),
                ("StateofChargeThreshold", 0x2A),
                ("LEDBlinkRate", 0x2B),
                ("OnTime", 0x2C),
                ("OffTime", 0x2D),
                ("LEdDisable", 0x2E),
            ]))

            self.SMBDAT_COMMAND_TYPE.update({
                "CycleCountThreshold": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "CapacityRatio": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "StateofChargeThreshold": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "LEDBlinkRate": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "OnTime": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "OffTime": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
                "LEdDisable": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
            })

        self.SMBDAT_COMMANDS.update(OrderedDict([
            # ("LifetimeData", 0x37),
            ("CellModelNo", 0x3C),
            ("Barcode", 0x3D),
            ("ModelNumber", 0x3E)
        ]))
        
        self.SMBDAT_COMMAND_TYPE.update({
            "LifetimeData": SmartBattery.SMB_BLK | SmartBattery.SMB_PEC,
            "CellModelNo": SmartBattery.SMB_BLK | SmartBattery.SMB_PEC,
            "Barcode": SmartBattery.SMB_BLK | SmartBattery.SMB_PEC,
            "ModelNumber": SmartBattery.SMB_BLK | SmartBattery.SMB_PEC,
        })

    def get_led_triggers(self):
        return self["CycleCountThreshold"], self["CapacityRatio"], self["StateofChargeThreshold"], self["LEdDisable"]
        
    def set_led_triggers(self, cycles, soh, soc, enable):
        self["CycleCountThreshold"] = cycles
        self["CapacityRatio"] = soh
        self["StateofChargeThreshold"] = soc
        self["LEdDisable"] = 0 if enable else 1
    
    def get_led_pattern(self):
        return self["LEDBlinkRate"], self["OnTime"], self["OffTime"]
    
    def set_led_pattern(self, rate, on, off):
        self["LEDBlinkRate"] = rate
        self["OnTime"] = on
        self["OffTime"] = off

def mask_test(val, msk):
    return (val & msk) != 0


class RolexBattery(PanasonicSmartBattery):
    BATTERY_ADDRESS = 0x0B

    def __init__(self, device_name, log, interface=None):
        super(RolexBattery, self).__init__(device_name, log, interface)

        self.SMBDAT_COMMANDS.update(OrderedDict([
            ("ChargeVoltage", 0x15),
            ("ManufacturerBlockAccess", 0x44),
            ("StateOfHealth", 0x4F),
        ]))

        self.SMBDAT_COMMAND_TYPE.update({
            "ChargeVoltage": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
            "ManufacturerBlockAccess": SmartBattery.SMB_BLK | SmartBattery.SMB_PEC,
            "StateOfHealth": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
        })
        
        self.device_number, self.version, self.build_number, *__ = self.get_firmware_info()
        if self.build_number >= 8:
            self.UPDATE_STATUS = 0x424E
            self.TERM_VOLTAGE = 0x4606
            self.TERM_VOLTAGE_DELTA = 0x4609
            self.SOH_LOAD_RATE = 0x4626
            self.LOAD_SELECT = 0x4688
            self.LOAD_MODE = 0x4689
            self.RESERVE_CAP_MAH = 0x4690
            self.SHUTDOWN_VOLTAGE = 0x4649
            self.SHUTDOWN_TIME = 0x464B
            self.DESIGN_CAPACITY_MAH = 0x465F
            self.DESIGN_VOLTAGE = 0x4665
            self.CYC_CNT_PCT = 0x4667
            self.SOC_FLAG_CONFIG_A = 0x4669
            self.SOC_FLAG_CONFIG_B = 0x466B
            self.FC_CLR_VOLTS = 0x4674
            self.FC_CLR_RSOC = 0x4677
            self.TCA_CLR_VOLTS = 0x4680
            self.TCA_CLR_RSOC = 0x4683    
            self.MAX_ERR_LIM = 0x4694   
            self.CHARGE_TERM_TAPER_CURRENT = 0x46CA
            self.CHARGE_TERM_VOLTAGE = 0x46CE
            self.TEMP_ENABLE = 0x46D1
            self.DSCRHG_CURRENT_THRESH = 0x46DD
            self.CHRGE_CURRENT_THRESH = 0x46DF
            self.QUIT_CURRENT_THRESH = 0x46E1
            self.PROTECTION_CONF = 0x46E5
            self.CUV_THRESH = 0x46EA    
            self.SBS_GAUGING_CONF = 0x4642
            self.IT_GAUGING_CONF = 0x4684
            self.ENABLED_PF_A = 0x4733
            self.ENABLED_PF_C = 0x4735
            self.BALANCING_CONF = 0x4741
            self.DataFlashITCfg = self.__DataFlashITCfgV8
            self.DataFlashGasGauge = self.__DataFlashGasGaugeV8

    def write_word(self, cmd, buff):
        return self.interface.send_write(cmd,
                                         Smbdat.SMB_WD | Smbdat.SMB_PEC,
                                         buff)
        
    # Alt Mfg Commands
    DEVICE_TYPE = 0x0001
    FIRMWARE_VERSION = 0x0002
    HARDWARE_VERSION = 0x0003
    CHEM_ID = 0x0006
    DSG_FET_CTRL = 0x0020
    GAUGE_EN = 0x0021
    FET_CTRL = 0x0022
    LIFETIME_DATA = 0x23
    PF_EN = 0x24
    SEAL_DEVICE = 0x30
    SECURITY_KEYS = 0x0035
    AUTHENTICATION_KEY = 0x0037
    DEVICE_RESET = 0x0041
    SAFETY_STATUS = 0x51
    PF_STATUS = 0x53
    OPERATION_STATUS = 0x0054
    CHARGING_STATUS = 0x0055
    GAUGING_STATUS = 0x0056
    MANUFACTURING_STATUS = 0x0057
    TERM_VOLT_CMD = 0x0059 # new in build 8 - ?
    DSG_CURR_ACCUM = 0x005B # new in build 8 - Discharge current accumulation
    QMAXCC = 0x005D     # new in build 8 - the cycle count that is recorded  at Qmax updates
    UP_TIME = 0x005E    # new in build 8 - Elapsed time in seconds
    DA_STATUS1 = 0x0071
    ROM_MODE = 0x0F00

    # Data Flash Addresses
    MFG_NAME = 0x40BF
    DEV_NAME = 0x40D4
    DATE_FIRST_USED = 0x4061
    CELL_MODEL_NO = 0x4076
    BARCODE = 0x408B
    MODEL_NO = 0x40A0
    DELTA_VOLTAGE = 0x4259
    DESIGN_RESISTANCE = 0x4240
    UPDATE_STATUS = 0x424E
    LED_CYCLE_CNT = 0x4380
    LED_CAP_RTO = 0x4382
    MFG_STATUS_INIT = 0x4407
    TERM_VOLTAGE = 0x45FE
    TERM_VOLTAGE_DELTA = 0x4601
    SOH_LOAD_RATE = 0x4628
    LOAD_SELECT = 0x4686
    LOAD_MODE = 0x4687
    RESERVE_CAP_MAH = 0x468C
    FET_OPTIONS = 0x4640
    POWER_CONFIG  = 0x4644
    SHUTDOWN_VOLTAGE = 0x4647
    SHUTDOWN_TIME = 0x4649
    DESIGN_CAPACITY_MAH = 0x465F
    DESIGN_VOLTAGE = 0x4663
    CYC_CNT_PCT = 0x4665
    SOC_FLAG_CONFIG_A = 0x4667
    SOC_FLAG_CONFIG_B = 0x4669
    FC_CLR_VOLTS = 0x4672
    FC_CLR_RSOC = 0x4675
    TCA_CLR_VOLTS = 0x467E
    TCA_CLR_RSOC = 0x4681    
    MAX_ERR_LIM = 0x4692    
    CHARGE_TERM_TAPER_CURRENT = 0x46CD
    CHARGE_TERM_VOLTAGE = 0x46D1
    TEMP_ENABLE = 0x46D4
    DSCRHG_CURRENT_THRESH = 0x46E0
    CHRGE_CURRENT_THRESH = 0x46E2
    QUIT_CURRENT_THRESH = 0x46E4
    PROTECTION_CONF = 0x46E8
    CUV_THRESH = 0x46ED    
    SBS_GAUGING_CONF = 0x4642
    IT_GAUGING_CONF = 0x4682
    ENABLED_PF_A = 0x4736
    ENABLED_PF_C = 0x4738
    BALANCING_CONF = 0x4744

    MFG_BLK_RD_NUM_TRIES = 5

    def df_rd(self, mem_addr):
        return self.mfg_blk_rd(mem_addr)
        
    def df_wd_rd(self, mem_addr):
        return struct.unpack('<H', self.df_rd(mem_addr)[:2])[0]
        
    def df_byte_rd(self, mem_addr):
        return struct.unpack('<B', self.df_rd(mem_addr)[:1])[0]
        
    PF_STATUS_FLAGS = {'VIMA': 0x00001000,
                       'VIMR': 0x00000800,
                       'SOV': 0x00000002,
                       'DFW': 0x04000000,
                       'IFC': 0x01000000,
                       'DFETF': 0x00020000,
                       'CFETF': 0x00010000}
    
    @property
    def pf_status(self):
        pf_stat = self.mfg_blk_rd(self.PF_STATUS)
        pf_stat = struct.unpack('<L', pf_stat)[0]
        return self.fields_to_list(self.PF_STATUS_FLAGS, pf_stat)        
        
    def mfg_blk_rd(self, cmd):
        if type(cmd) is int:
            cmd = struct.pack('<H', cmd)
        
        assert type(cmd) is bytes
        assert len(cmd) == 2
    
        response = None
        for i in range(self.MFG_BLK_RD_NUM_TRIES):
            try:
                self['ManufacturerBlockAccess'] = cmd
                response = self['ManufacturerBlockAccess']
                expected = struct.unpack('<H', cmd[:2])[0]
                got = struct.unpack('<H', response[:2])[0]
                assert expected == got, "Mfg Command mismatch, expected(%04x) got(%04x)" % (expected, got)
            except (AssertionError, ValueError, IOError) as e:
                self.log.error(e)
                self.send_reset()
                time.sleep(0.5)
                if i == (self.MFG_BLK_RD_NUM_TRIES - 1):
                    raise e
            else:
                break

        return response[2:]

    # def df_wr(self, mem_addr, data):
        # if type(mem_addr) is not int:
            # mem_addr = struct.unpack('<H', mem_addr)[0]
        
        # assert type(mem_addr) is int
        # assert type(data) is bytearray
        
        # row_len = 32
        # row_addr = mem_addr & 0xFFE0
        # row_buffer = self.df_rd(row_addr)
        # offset = mem_addr & 0x001F
        
        # print(hex(mem_addr), hex(row_addr), hex(offset))
        
        # print(row_buffer.hex(), data.hex())
        
        # row_buffer[offset:offset+len(data)] = data
        
        # print(row_buffer.hex())
        
        # exit(1)
        
        # if type(mem_addr) is int:
            # buffer = bytearray(struct.pack('<H', mem_addr))
            # buffer.extend(data)
        # else:
            # buffer = bytearray(mem_addr)
            # buffer.extend(data)
        
        # self.alt_mfg_cmd(buffer)
        
    def df_wr(self, mem_addr, data):
        if type(mem_addr) is int:
            mem_addr = struct.pack('<H', mem_addr)
        
        assert type(mem_addr) is bytes
        assert len(mem_addr) == 2
        assert type(data) is bytearray
        
        if type(mem_addr) is int:
            buffer = bytearray(struct.pack('<H', mem_addr))
            buffer.extend(data)
            self.alt_mfg_cmd(buffer)
        else:
            buffer = bytearray(mem_addr)
            buffer.extend(data)
            self.alt_mfg_cmd(buffer)

    def alt_mfg_cmd(self, cmd):
        if type(cmd) is int:
            cmd = struct.pack('<H', cmd)
        if type(cmd) is bytearray:
            cmd = bytes(cmd)
#lou        
#        assert (type(cmd) is bytes), F'cmd is type {type(cmd)}'
        assert ("type(cmd) is bytes:")

        # don't assert length since `cmd` may contain payload
    
        for i in range(5):
            try:
                self['ManufacturerBlockAccess'] = cmd
            except (AssertionError, ValueError, IOError) as e:
                self.log.error(e)
                time.sleep(0.5)
                if i == 4:
                    raise e
            else:
                break

    def send_key(self, key_index, key_data):
        buffer = struct.pack('<BBBB', 0, 0, 0, key_index)
        self.interface.raw_write(buffer)
        time.sleep(0.1)
        self['ManufacturerAccess'] = self.AUTHENTICATION_KEY
        self['Authentication'] = key_data
        time.sleep(0.1)
                
    IT_STAT_QMAXDODOK = 0x2000  # New in build 8 - Flatzone is indicated when the new bit in It Status is cleared
    IT_STAT_OCVFR = 0x1000
    IT_STAT_LDMD = 0x0800
    IT_STAT_RX = 0x0400
    IT_STAT_QMAX = 0x0200
    IT_STAT_VDQ = 0x0100
    IT_STAT_NSFM = 0x0080
    IT_STAT_OCVPRED = 0x0040  # New in build 8 - OCV Prediction status
    IT_STAT_SLPQ = 0x0020
    IT_STAT_QEN = 0x0010
    IT_STAT_VOK = 0x0008
    IT_STAT_RDIS = 0x0004
    IT_STAT_REST = 0x0001

    GA_STAT_DSG = 0x00000040
    GA_STAT_EDV = 0x00000020
    GA_STAT_BAL_EN = 0x00000010
    GA_STAT_TCA = 0x00000008
    GA_STAT_TDA = 0x00000004
    GA_STAT_FC = 0x00000002
    GA_STAT_FD = 0x00000001

    def it_status_reg(self):
        # the TRM and BQStudio disagree here so lets get to BQStudio
        g_stat = self.mfg_blk_rd(self.GAUGING_STATUS)
        return struct.unpack('<BH', g_stat)[1]

    def gauging_status_reg(self):
        # the TRM and BQStudio disagree here so lets get to BQStudio
        g_stat = self.mfg_blk_rd(self.GAUGING_STATUS)
        return struct.unpack('<BH', g_stat)[0]

    IT_STATUS_FLAGS = {'QMAXDODOK' : 0x2000,    # New in build 8 - Flatzone is indicated when the new bit in It Status is cleared
                       'OCVFR': 0x1000,
                       'LDMD': 0x0800,
                       'RX': 0x0400,
                       'QMAX': 0x0200,
                       'VDQ': 0x0100,
                       'NSFM': 0x0080,
                       'OCVPRED' : 0x0040,  # New in build 8 - OCV Prediction status
                       'SLPQ': 0x0020,
                       'QEN': 0x0010,
                       'VOK': 0x0008,
                       'RDIS': 0x0004,
                       'REST': 0x0001}    # Enhanced in build 8 - OCV Prediction status
    
    @property
    def it_status(self):
        it_status = struct.unpack('<BH', self.mfg_blk_rd(self.GAUGING_STATUS))[1]
        return self.fields_to_list(self.IT_STATUS_FLAGS, it_status)            
        
    GAUGING_STATUS_FLAGS = {'DSG': 0x00000040,
                            'EDV': 0x00000020,
                            'BAL_EN': 0x00000010,
                            'TCA': 0x00000008,
                            'TDA': 0x00000004,
                            'FC': 0x00000002,
                            'FD': 0x00000001}
    
    @property
    def gauging_status(self):
        gauging_status = struct.unpack('<BH', self.mfg_blk_rd(self.GAUGING_STATUS))[0]
        return self.fields_to_list(self.GAUGING_STATUS_FLAGS, gauging_status)        
        
    def fet_ctrl(self, enable):
        mfg_stat = self.manufacturing_status()
        if enable != mfg_stat['FET']:
            self.alt_mfg_cmd(self.FET_CTRL)
        mfg_stat = self.manufacturing_status()
        self.log.info('manufacturing_status().FET(%s)', mfg_stat['FET'])

    def get_firmware_info(self):
        device_number, version, build_number, fw_type, it_ver, rsvd = struct.unpack('>HHHHHB', self.mfg_blk_rd(self.FIRMWARE_VERSION))
        return (device_number, version, build_number, fw_type, it_ver, rsvd)
        
    MFG_STAT_CALIBRATION = 0x8000
    MFG_STAT_PF = 0x0040
    MFG_STAT_LF = 0x0020
    MFG_STAT_FET = 0x0010
    MFG_STAT_GAUGE = 0x0008
    MFG_STAT_DSG = 0x0004
    MFG_STAT_CHG = 0x0002

    def manufacturing_status(self):
        mfg_stat = struct.unpack('<H', self.mfg_blk_rd(self.MANUFACTURING_STATUS))[0]
        return {
            'calibration': mask_test(mfg_stat, self.MFG_STAT_CALIBRATION),
            'permanent_failure': mask_test(mfg_stat, self.MFG_STAT_PF),
            'lifetime': mask_test(mfg_stat, self.MFG_STAT_LF),
            'FET': mask_test(mfg_stat, self.MFG_STAT_FET),
            'gauge': mask_test(mfg_stat, self.MFG_STAT_GAUGE),
            'dsg_fet_test': mask_test(mfg_stat, self.MFG_STAT_DSG),
            'chg_fet_test': mask_test(mfg_stat, self.MFG_STAT_CHG)
        }

    class DaStat1(Struct):
        _format = Format.LittleEndian

        cell_mv_1 = Type.Short
        cell_mv_2 = Type.Short
        rsvd_1 = Type.UnsignedShort
        rsvd_2 = Type.UnsignedShort
        bat_mv = Type.Short
        pack_mv = Type.Short
        cell_ma_1 = Type.Short
        cell_ma_2 = Type.Short
        rsvd_3 = Type.UnsignedShort
        rsvd_4 = Type.UnsignedShort
        cell_mw_1 = Type.Short
        cell_mw_2 = Type.Short
        rsvd_5 = Type.UnsignedShort
        rsvd_6 = Type.UnsignedShort
        power_calc = Type.Short
        power_avg_calc = Type.Short

        @classmethod
        def read_pack(cls, pack):
            return cls(pack.mfg_blk_rd(pack.DA_STATUS1))

    def security_mode(self):
        op_stat = struct.unpack('<L', self.mfg_blk_rd(self.OPERATION_STATUS))[0]
        return (op_stat & 0x300) >> 8

    def device_reset(self):
        self.alt_mfg_cmd(self.DEVICE_RESET)
        self.send_reset()
        time.sleep(5)
        self.mfg_blk_rd(self.DEVICE_TYPE)

    SECURITY_MODE_SEALED = 3
    SECURITY_MODE_UNSEAL = 2
    SECURITY_MODE_FULL = 1
    DEFAULT_UNSEAL_KEY = (0x0414, 0x3672)
    DEFAULT_FULL_ACCESS_KEY = (0xFFFF, 0xFFFF)
    ZEBRA_FULL_ACCESS_KEY = (0xEDFE, 0x0DF0)

    def unseal_device(self, key=(0x0414, 0x3672)):
        self['ManufacturerAccess'] = key[0]
        self['ManufacturerAccess'] = key[1]
        assert(self.security_mode() == self.SECURITY_MODE_UNSEAL)

    BATTERY_STATUS_EC0 = 0x0001
    BATTERY_STATUS_EC1 = 0x0002
    BATTERY_STATUS_EC2 = 0x0004
    BATTERY_STATUS_EC3 = 0x0008
    BATTERY_STATUS_FD = 0x0010
    BATTERY_STATUS_FC = 0x0020
    BATTERY_STATUS_DSG = 0x0040
    BATTERY_STATUS_INIT = 0x0080
    BATTERY_STATUS_RTA = 0x0100
    BATTERY_STATUS_RCA = 0x0200
    BATTERY_STATUS_TDA = 0x0800
    BATTERY_STATUS_OTA = 0x1000
    BATTERY_STATUS_TCA = 0x4000
    BATTERY_STATUS_OCA = 0x8000

    @staticmethod
    def fields_to_list(flag_dict, field_reg):
         for name, mask in flag_dict.items():
#             print(F'{field_reg:x} {mask:x} {(field_reg & mask):x} {((field_reg & mask) != 0)}')
               print(field_reg,mask,field_reg & mask,field_reg & mask)
    
    #         print(F'{flag_dict!r} {field_reg!r}')
               print(flag_dict, field_reg)
#    return [name for name, mask in flag_dict.items() if ((field_reg & mask) != 0)]
    
    BATTERY_STATUS_FLAGS = {'error_code': 0x000F,
                            'FD': 0x0010,
                            'FC': 0x0020,
                            'DSG': 0x0040,
                            'INIT': 0x0080,
                            'RTA': 0x0100,
                            'RCA': 0x0200,
                            'TDA': 0x0800,
                            'OTA': 0x1000,
                            'TCA': 0x4000,
                            'OCA': 0x8000}
    
    @property
    def battery_status(self):
        battery_status = self['BatteryStatus']
        return self.fields_to_list(self.BATTERY_STATUS_FLAGS, battery_status)
    
    SAFETY_STATUS_FLAGS = {'CUV': 0x00000001,
                           'COV': 0x00000002,
                           'OCC': 0x00000004,
                           'OCD': 0x00000010,
                           'AOLD': 0x00000040,
                           'ASCC': 0x00000100,
                           'ASCD': 0x00000400,
                           'OTC': 0x00001000,
                           'OTD': 0x00002000,
                           'PTO': 0x00040000,
                           'CTO': 0x00100000,
                           'UTC': 0x04000000,
                           'UTD': 0x08000000}

    @property
    def safety_status(self):
        safety_status = struct.unpack('<L', self.mfg_blk_rd(self.SAFETY_STATUS))[0]
        return self.fields_to_list(self.SAFETY_STATUS_FLAGS, safety_status)
    
    OPERATION_STATUS_FLAGS = {'DSG': 0x00000002,
                              'CHG': 0x00000003,
                              'BTP_INT': 0x00000080,
                              'SEC': 0x00000300,
                              'SDV': 0x00000400,
                              'SS': 0x00000800,
                              'PF': 0x00001000,
                              'XDSG': 0x00002000,
                              'XCHG': 0x00004000,
                              'SLEEP': 0x00008000,
                              'SDM': 0x00010000,
                              'AUTH': 0x00040000,
                              'AUTOCALM': 0x00080000,
                              'CAL': 0x00100000,
                              'CAL_OFFSET': 0x00200000,
                              'XL': 0x00400000,
                              'SLEEPM': 0x00800000,
                              'INIT': 0x01000000,
                              'SMBLCAL': 0x02000000,
                              'SLPAD': 0x04000000,
                              'SLPCC': 0x08000000,
                              'CB': 0x10000000,
                              'EMSHUT': 0x20000000}

    @property
    def operation_status(self):
        operation_status = struct.unpack('<L', self.mfg_blk_rd(self.OPERATION_STATUS))[0]
        return self.fields_to_list(self.OPERATION_STATUS_FLAGS, operation_status)
        
    CHARGING_STATUS_FLAGS = {'PV': 0x01,
                             'LV': 0x02,
                             'MV': 0x04,
                             'HV': 0x08,
                             'IN': 0x10,
                             'SU': 0x20,
                             'MCHG': 0x40,
                             'VCT': 0x80}

    @property
    def charging_status(self):
        charging_status = struct.unpack('<H', self.mfg_blk_rd(self.CHARGING_STATUS)[1:])[0]
        return self.fields_to_list(self.CHARGING_STATUS_FLAGS, charging_status)    
    
    CHARGE_STATUS_VCT = 0x80

    def read_flash_data(self, start_addr, end_addr):
        flash_data_buffer = bytearray()
        for i in range(self.MFG_BLK_RD_NUM_TRIES):
            try:
                address = start_addr
                self['ManufacturerBlockAccess'] = struct.pack('<H', address)

                while address < end_addr:
                    response = self['ManufacturerBlockAccess']
                    got = struct.unpack('<H', response[:2])[0]
                    assert address == got, "Mfg Command mismatch, expected(%04x) got(%04x)" % (address, got)
                    remaining_bytes = (end_addr - address) + 1
                    if remaining_bytes >= len(response[2:]):
                        flash_data_buffer.extend(response[2:])
                         print(F'Read {got:#06x} : {response[2:].hex()}')
                    else:
                        flash_data_buffer.extend(response[2:(remaining_bytes+2)])
                         print(F'Read {got:#06x} : {response[2:(remaining_bytes+2)].hex()}')
                    address += len(response[2:])

            except (AssertionError, ValueError, IOError) as exc:
                self.log.error(exc)
                self.send_reset()
                time.sleep(0.5)
                if i == (self.MFG_BLK_RD_NUM_TRIES - 1):
                    raise exc
            else:
                break

        self.log.debug('read_flash_data() returned %d bytes', len(flash_data_buffer))

        return flash_data_buffer
        
    class DataFlashITCfg(Struct):
        _format = Format.LittleEndian
        design_resistance = Type.Short
        pack_resistance = Type.Short
        system_resistance = Type.Short
        rsvd_1 = ArrayElement(Type.UnsignedByte, 0x0388)
        ra_filter = Type.UnsignedShort
        rsvd_2 = Type.UnsignedByte
        ra_max_delta = Type.UnsignedByte
        rsvd_3 = Type.UnsignedShort
        r_param_filter = Type.UnsignedShort
        edv_r_param_filter = Type.UnsignedShort
        rsvd_4 = ArrayElement(Type.UnsignedByte, 36)
        qmax_delta = Type.UnsignedByte
        qmax_ceiling = Type.UnsignedByte
        term_voltage = Type.Short
        term_voltage_hold = Type.UnsignedByte
        term_voltage_delta = Type.Short
        term_min_cell_voltage = Type.Short
        rsvd_5 = ArrayElement(Type.UnsignedByte, 7)
        max_sim_iter = Type.UnsignedByte
        rsvd_6 = ArrayElement(Type.UnsignedByte, 6)
        fast_scale_start = Type.UnsignedByte
        rsvd_7 = ArrayElement(Type.UnsignedByte, 8)
        delta_voltage_min = Type.Short
        rsvd_8 = ArrayElement(Type.UnsignedByte, 0x62)
        load_select = Type.UnsignedByte
        load_mode = Type.UnsignedByte
        user_rate_ma = Type.Short
        user_rate_cw = Type.Short
        reserve_cap_ma = Type.Short
        reserve_cap_cw = Type.Short

        @classmethod
        def read_pack(cls, pack):
            try:
                buffer = pack.read_flash_data(pack.DESIGN_RESISTANCE, pack.DESIGN_RESISTANCE+len(cls)-1)
                return cls(buffer)            
            except struct.error:
#lou

                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
                #print("Unable to unpack buffer length expected ? got ?")
                raise
                
    class __DataFlashITCfgV8(Struct):
        _format = Format.LittleEndian
        design_resistance = Type.Short
        pack_resistance = Type.Short
        system_resistance = Type.Short
        rsvd_1 = ArrayElement(Type.UnsignedByte, 0x0388)
        ra_filter = Type.UnsignedShort
        rsvd_2 = Type.UnsignedByte
        ra_max_delta = Type.UnsignedByte
        rsvd_3 = Type.UnsignedShort
        r_param_filter = Type.UnsignedShort
        edv_r_param_filter = Type.UnsignedShort
        rsvd_4 = ArrayElement(Type.UnsignedByte, 36)
        qmax_delta = Type.UnsignedByte
        qmax_ceiling = Type.UnsignedByte
        cycle_adjust_thresh = Type.UnsignedByte
        cycle_adjust_incr = Type.UnsignedByte
        ocv_pred_active_t_lim = Type.UnsignedShort
        ocv_pred_transient_t = Type.UnsignedShort
        ocv_pred_measure_e = Type.UnsignedShort
        term_voltage = Type.Short
        term_voltage_hold = Type.UnsignedByte
        term_voltage_delta = Type.Short
        term_min_cell_voltage = Type.Short
        rsvd_5 = ArrayElement(Type.UnsignedByte, 7)
        max_sim_iter = Type.UnsignedByte
        rsvd_6 = ArrayElement(Type.UnsignedByte, 6)
        fast_scale_start = Type.UnsignedByte
        rsvd_7 = ArrayElement(Type.UnsignedByte, 8)
        delta_voltage_min = Type.Short
        rsvd_8 = ArrayElement(Type.UnsignedByte, 0x62)
        load_select = Type.UnsignedByte
        load_mode = Type.UnsignedByte
        user_rate_ma = Type.Short
        user_rate_cw = Type.Short
        reserve_cap_ma = Type.Short
        reserve_cap_cw = Type.Short

        @classmethod
        def read_pack(cls, pack):
            try:
                buffer = pack.read_flash_data(pack.DESIGN_RESISTANCE, pack.DESIGN_RESISTANCE+len(cls)-1)
                return cls(buffer)            
            except struct.error:
#lou
                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
                #print("Unable to unpack buffer length expected ? got ?")
                raise
    @property
    def df_it_cfg(self):
        return self.DataFlashITCfg.read_pack(self)         
    
    class DataFlashGasGauge(Struct):
        _format = Format.LittleEndian
        design_resistance = Type.Short
        pack_resistance = Type.Short
        system_resistance = Type.Short
        qmax_cell_1 = Type.Short
        qmax_cell_2 = Type.Short
        qmax_pack = Type.Short
        qmax_cycle_count = Type.UnsignedShort
        update_status = Type.Byte
        cell_1_chg_voltage_at_EoC = Type.Short
        cell_2_chg_voltage_at_EoC = Type.Short
        current_at_EoC = Type.Short
        avg_i_last_run = Type.Short
        avg_p_last_run = Type.Short
        delta_voltage = Type.Short
        temp_k = Type.Short
        temp_a = Type.Short
        max_avg_i_last_run = Type.Short
        max_avg_p_last_run = Type.Short
        rsvd_0 = ArrayElement(Type.UnsignedByte, 29)
        cycle_count = Type.UnsignedShort
        rsvd_1 = ArrayElement(Type.UnsignedByte, 844)
        ra_filter = Type.UnsignedShort
        rsvd_2 = ArrayElement(Type.UnsignedByte, 1)
        ra_max_delta = Type.UnsignedByte
        resistance_parameter_filter = Type.UnsignedShort
        near_edv_ra_param_filter = Type.UnsignedShort
        rsvd_3 = ArrayElement(Type.UnsignedByte, 38)
        qmax_delta = Type.UnsignedByte
        qmax_upper_bound = Type.UnsignedByte
        term_voltage = Type.Short
        term_v_hold_time = Type.UnsignedByte
        term_voltage_delta = Type.Short
        term_min_cell_v = Type.Short
        rsvd_4 = ArrayElement(Type.UnsignedByte, 7)
        max_simulation_iterations = Type.UnsignedByte
        rsvd_5 = ArrayElement(Type.UnsignedByte, 16)
        fast_scale_start_soc = Type.UnsignedByte
        rsvd_6 = ArrayElement(Type.UnsignedByte, 8)
        min_delta_voltage = Type.Short
        soh_load_rate = Type.UnsignedByte

        @classmethod
        def read_pack(cls, pack):
            return cls(pack.read_flash_data(pack.DESIGN_RESISTANCE, pack.DESIGN_RESISTANCE+len(cls)-1))
    
    class __DataFlashGasGaugeV8(Struct):
        _format = Format.LittleEndian
        design_resistance = Type.Short
        pack_resistance = Type.Short
        system_resistance = Type.Short
        qmax_cell_1 = Type.Short
        qmax_cell_2 = Type.Short
        qmax_pack = Type.Short
        qmax_cycle_count = Type.UnsignedShort
        update_status = Type.Byte
        cell_1_chg_voltage_at_EoC = Type.Short
        cell_2_chg_voltage_at_EoC = Type.Short
        current_at_EoC = Type.Short
        avg_i_last_run = Type.Short
        avg_p_last_run = Type.Short
        delta_voltage = Type.Short
        temp_k = Type.Short
        temp_a = Type.Short
        max_avg_i_last_run = Type.Short
        max_avg_p_last_run = Type.Short
        rsvd_0 = ArrayElement(Type.UnsignedByte, 29)
        cycle_count = Type.UnsignedShort
        rsvd_1 = ArrayElement(Type.UnsignedByte, 844)
        ra_filter = Type.UnsignedShort
        rsvd_2 = ArrayElement(Type.UnsignedByte, 1)
        ra_max_delta = Type.UnsignedByte
        resistance_parameter_filter = Type.UnsignedShort
        near_edv_ra_param_filter = Type.UnsignedShort
        rsvd_3 = ArrayElement(Type.UnsignedByte, 38)
        qmax_delta = Type.UnsignedByte
        qmax_upper_bound = Type.UnsignedByte
        cycle_adjust_threshold = Type.UnsignedByte
        cycle_adj_incr = Type.UnsignedByte
        ocv_pred_active_t_limit = Type.UnsignedShort
        ocv_pred_transient_t = Type.UnsignedShort
        ocv_pred_measure_time = Type.UnsignedShort
        term_voltage = Type.Short
        term_v_hold_time = Type.UnsignedByte
        term_voltage_delta = Type.Short
        term_min_cell_v = Type.Short
        rsvd_4 = ArrayElement(Type.UnsignedByte, 7)
        max_simulation_iterations = Type.UnsignedByte
        rsvd_5 = ArrayElement(Type.UnsignedByte, 6)
        fast_scale_start_soc = Type.UnsignedByte
        rsvd_6 = ArrayElement(Type.UnsignedByte, 8)
        min_delta_voltage = Type.Short
        soh_load_rate = Type.UnsignedByte
        soh_temp_k = Type.Short
        soh_temp_a = Type.Short

        @classmethod
        def read_pack(cls, pack):
            return cls(pack.read_flash_data(pack.DESIGN_RESISTANCE, pack.DESIGN_RESISTANCE+len(cls)-1))

    @property
    def df_gg_status(self):
        return self.DataFlashGasGauge.read_pack(self)             
    
    class DataFlashLifetimes(Struct):
        _format = Format.LittleEndian
        cell_1_max_voltage = Type.Short
        cell_2_max_voltage = Type.Short
        max_charge_current = Type.Short
        max_discharge_current = Type.Short
        max_temp_cell = Type.Byte
        min_temp_cell = Type.Byte

        @classmethod
        def read_pack(cls, pack):
            return cls(pack.read_flash_data(0x42C0, 0x42C9))

    @property
    def df_ra_table(self):
        return self.DataFlashRa.read_pack(self) 
            
    class DataFlashRa(Struct):
        _format = Format.LittleEndian
        cell_0_R_a_flag = Type.Short
        cell_0_R_a_0 = Type.Short
        cell_0_R_a_1 = Type.Short
        cell_0_R_a_2 = Type.Short
        cell_0_R_a_3 = Type.Short
        cell_0_R_a_4 = Type.Short
        cell_0_R_a_5 = Type.Short
        cell_0_R_a_6 = Type.Short
        cell_0_R_a_7 = Type.Short
        cell_0_R_a_8 = Type.Short
        cell_0_R_a_9 = Type.Short
        cell_0_R_a_10 = Type.Short
        cell_0_R_a_11 = Type.Short
        cell_0_R_a_12 = Type.Short
        cell_0_R_a_13 = Type.Short
        cell_0_R_a_14 = Type.Short
        cell_1_R_a_flag = Type.Short
        cell_1_R_a_0 = Type.Short
        cell_1_R_a_1 = Type.Short
        cell_1_R_a_2 = Type.Short
        cell_1_R_a_3 = Type.Short
        cell_1_R_a_4 = Type.Short
        cell_1_R_a_5 = Type.Short
        cell_1_R_a_6 = Type.Short
        cell_1_R_a_7 = Type.Short
        cell_1_R_a_8 = Type.Short
        cell_1_R_a_9 = Type.Short
        cell_1_R_a_10 = Type.Short
        cell_1_R_a_11 = Type.Short
        cell_1_R_a_12 = Type.Short
        cell_1_R_a_13 = Type.Short
        cell_1_R_a_14 = Type.Short
        xcell_0_R_a_flag = Type.Short
        xcell_0_R_a_0 = Type.Short
        xcell_0_R_a_1 = Type.Short
        xcell_0_R_a_2 = Type.Short
        xcell_0_R_a_3 = Type.Short
        xcell_0_R_a_4 = Type.Short
        xcell_0_R_a_5 = Type.Short
        xcell_0_R_a_6 = Type.Short
        xcell_0_R_a_7 = Type.Short
        xcell_0_R_a_8 = Type.Short
        xcell_0_R_a_9 = Type.Short
        xcell_0_R_a_10 = Type.Short
        xcell_0_R_a_11 = Type.Short
        xcell_0_R_a_12 = Type.Short
        xcell_0_R_a_13 = Type.Short
        xcell_0_R_a_14 = Type.Short
        xcell_1_R_a_flag = Type.Short
        xcell_1_R_a_0 = Type.Short
        xcell_1_R_a_1 = Type.Short
        xcell_1_R_a_2 = Type.Short
        xcell_1_R_a_3 = Type.Short
        xcell_1_R_a_4 = Type.Short
        xcell_1_R_a_5 = Type.Short
        xcell_1_R_a_6 = Type.Short
        xcell_1_R_a_7 = Type.Short
        xcell_1_R_a_8 = Type.Short
        xcell_1_R_a_9 = Type.Short
        xcell_1_R_a_10 = Type.Short
        xcell_1_R_a_11 = Type.Short
        xcell_1_R_a_12 = Type.Short
        xcell_1_R_a_13 = Type.Short
        xcell_1_R_a_14 = Type.Short

        @classmethod
        def read_pack(cls, pack):
            buffer_data = pack.read_flash_data(0x4140, 0x415F)
            buffer_data.extend(pack.read_flash_data(0x4180, 0x419F))
            buffer_data.extend(pack.read_flash_data(0x41C0, 0x41DF))
            buffer_data.extend(pack.read_flash_data(0x4200, 0x421F))
            return cls(buffer_data)
    '''
    def to_srec(self, filename):
        """
        Section 1 : DF : S32800004000 - S30600004FFF    4000-5FFF were read (4000-4FFF appear to mirrror 5000-5FFF)
        Section 2 : IF : S32800100000 - S30D00107FF8    0000-DFFF were read
        Section 3 :    : S32800140000 - S31B001401EA    0000-01FF were read
        """
        ROM_DEVICE_ADDRESS = 0x0B
        
        IF_ADDR_CMD = 0x00
        IF_RDROW_CMD = 0x03
        DF_ADDR_CMD = 0x09
        DF_RDROW_CMD = 0x0C
        BLK_EN_CMD = 0x1A
        
        def rom_write_word(command_code, data):
            # print('WW %#04x %#06x'%(command_code, data))
            # return
            retval =  self.interface.write_word(ROM_DEVICE_ADDRESS, command_code, struct.pack("<H", data), use_pec=True)
            time.sleep(0.001)
        
        def rom_read_blk(command_code):
            # print('RW %#04x' % (command_code,))
            # return 0x9101

            retval =  self.interface.read_word(ROM_DEVICE_ADDRESS,
                                               command_code,
                                               use_pec=True)
            return struct.unpack("<H", retval)[0]
            
        def rom_read_section1_row(addr):
            rom_write_word(DF_ADDR_CMD, addr)
            return rom_read_blk(DF_RDROW_CMD)
        
        def rom_read_section2_row(addr):
            rom_write_word(IF_ADDR_CMD, addr)
            return rom_read_blk(IF_RDROW_CMD)
        
        def rom_read_section3_row(addr):
            rom_write_word(BLK_EN_CMD, 0x83de)
            rom_write_word(IF_ADDR_CMD, addr)
            return rom_read_blk(IF_RDROW_CMD)
    
        SECTION1_BASE = 0x4000
        SECTION2_BASE = 0x100000
        SECTION3_BASE = 0x140000
        section1_size = 0x1000
        section2_size = 0xE000
        section3_size = 0x200
    
        section_info = [{'start':SECTION1_BASE, 'size':section1_size, 'read_func':rom_read_section1_row},
                        {'start':SECTION2_BASE, 'size':section2_size, 'read_func':rom_read_section2_row},
                        {'start':SECTION3_BASE, 'size':section3_size, 'read_func':rom_read_section3_row}]
    
        srec_data = "S01C000000000000000000000000000000000000000000000000000000E3"
    
        for section in section_info:
            buffer = bytearray([0xFF] * section['size'])
            addr = section['start']
            last_row_used = addr
            
            while addr < section['start'] + section['size']:
                row_data = section['read_func'](addr)[1:row_data[0]+1] # skip the length and PEC bytes
#lou
                struct.pack_into(F'{len(row_data)}s', buffer, addr - section['start'], row_data)
#                struct.pack_into("'?buffer, addr - section ?")

                addr += len(row_data)
                
                if ([0xFF] * len(row_data)) not in row_data:
                    last_row_used = addr
                    
            buffer = buffer[:last_row_used]
            
            offset = 0
            while offset < len(buffer):
                row_len = min(0x28, len(buffer) - offset)
                row_data = buffer[offset:offset+row_len]
                crc = 
                srec_data += F'S3{row_len:02X}{(section["start"]+offset):08X}{row_data.hex()}{crc:02X}\n'
                offset += row_len
    
    '''    
    def program_srec(self, srec):

        ROM_DEVICE_ADDRESS = 0x0B
    
        def rom_write_block(command_code, data):
            device_address = ROM_DEVICE_ADDRESS
            string_data = bytearray([device_address << 1])
            string_data.append(command_code)
            string_data.append(len(data))
            string_data.extend(data)

            # print('WB %s' % (':'.join([hex(x) for x in string_data[1:]])))
            # return

            rc = self.interface.write_bytearray(device_address, string_data[1:])

            if rc != self.interface.CY_SUCCESS:
                self.interface.i2c_dev.reset(1)
                raise IOError('Failed to write block')

            return rc

        def rom_read_word(command_code):
            # print('RW %#04x' % (command_code,))
            # return 0x9101

            retval =  self.interface.read_word(ROM_DEVICE_ADDRESS,
                                               command_code,
                                               use_pec=True)
            return struct.unpack("<H", retval)[0]

        def rom_write_word(command_code, data):
            # print('WW %#04x %#06x'%(command_code, data))
            # return
            retval =  self.interface.write_word(ROM_DEVICE_ADDRESS, command_code, struct.pack("<H", data), use_pec=True)
            time.sleep(0.001)

        def rom_write_cmd(command_code):
            # print('WC %#04x'%(command_code,))
            # return
            if type(command_code) is int:
                command_code = bytearray([command_code])
            return self.interface.write_bytearray(ROM_DEVICE_ADDRESS, command_code)

        section4_data = [0] * 4
        SECTION1_BASE = 0x4000
        SECTION2_BASE = 0x100000
        SECTION3_BASE = 0x140000
        section1_size = 0x08A0 # 8*1024
        section2_size = 0x7FA0 # 56*1024
        section3_size = 512

        '''
        'ROM Interface
        '''
        # Read IF
        # Command 0x00 Set flash address (WriteBlock): 3 byte block. First two bytes are IF row address (little endian) 3rd byte is column address
        # Command 0x01 Read IF word (ReadBlock): 3 byte block, IF word at specified address, little endian. Column address auto-increments after read
        # Command 0x02 Read IF row (ReadBlock): 96 byte block, 32 IF words from specified row.
        # Command 0x03 IF row sum (ReadBlock): 4 byte block, 32 bit sum of 32 IF words on specified row.

        # Prog IF
        # Command 0x04 Prog IF word (WriteBlock): 6 byte block, First two bytes  are IF row (little endian) 3rd byte is column, last 3 bytes are IF  word (little endian)
        # Command 0x05 Prog IF row (WriteBlock):  98 byte block, First two bytes are IF row (little endian), last 96 bytes are 32 IF words (each word little endian)
        # Command 0x06 Erase IF row (WriteWord): Word is the IF row to erase  
        # Command 0x07 Mass erase IF (WriteWord): Word must be 0x83de

        # Execute IF:
        # Command 0x08 Execute (Command): Execute firmware (ignores integrity status)

        # Read RAM/DF:
        # Command 0x09 Set address (WriteWord): 2 byte data address (DF starts  at 0x4000)
        # Command 0x0a Poke byte (WriteWord): Write LSB to address  (MSB ignored)
        # Command 0x0b Peek byte (ReadWord): LSB is data from specified address (MSB undefined)
        # Command 0x0c Read row (ReadBlock): 32 bytes of data starting at specified address

        # Version:
        # Command 0x0d Version (ReadWord): Reports ROM version

        # Prog DF:
        # Command 0x0e DF sum (ReadWord): Sum of all user DF (last 64 bytes of DF are reserved, not included in sum)
        # Command 0x0f Prog DF word (WriteBlock): 3 byte block, First two bytes are full DF address (starting at 0x4000), last byte is data word.
        # Command 0x10 Prog DF row (WriteBlock): 33 byte block, First byte is row address, last 32 bytes are row data words
        # Command 0x11 Erase DF  row (WriteWord): LSB is row number to erase  
        # Command 0x12 Mass erase DF (WriteWord): Word must be 0x83de.


        IF_ADDR_CMD = 0x00
        
        IF_WRITE_CMD = 0x05     # Write IF all
        PG_ERASE_CMD = 0x06     # Erase IF Row
        IF_ERASE_CMD = 0x07     # Erase IF all
        
        DF_ADDR_CMD = 0x09
        DF_POKE_CMD = 0x0A
        DF_PEEK_CMD = 0x0B
        
        DF_WRITE_CMD = 0x0f
        DF_ERASE_CMD = 0x11
        BLK_EN_CMD = 0x1A
        data_array_index = 0

        with open(srec) as f:
            testlist_array = list(f)

        # Jump to ROM loader
        
        try:
            self.alt_mfg_cmd(self.ROM_MODE)
        except CY_ERROR_I2C_NAK_ERROR:
            pass

        time.sleep(0.01)
        
        rom_version = rom_read_word(0x0D)
        self.log.info('Read word 0x0D : %#04X', rom_version)

        assert rom_version == 0x9101, F'Part {rom_version:#06x} is incompatible with programming'

        self.log.info("Erasing...")
        '''
        ' BQStudio does this and so will we
        '''
        # rom_write_word(DF_ADDR_CMD, 0)
        # jump_vector = rom_read_word(DF_PEEK_CMD)
        # rom_write_word(DF_POKE_CMD, 0x000A)
        
        # rom_write_word(DF_ADDR_CMD, 0x0002)
        # rom_write_word(DF_POKE_CMD, 0)
        # rom_write_word(IF_ADDR_CMD, 0)
        
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_write_word(PG_ERASE_CMD, 0)
        # time.sleep(0.5)
        
        # rom_write_word(IF_ADDR_CMD, 0x0080)
        
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_write_word(PG_ERASE_CMD, 0x0080)
        # time.sleep(0.5)
        
        # rom_write_word(DF_ADDR_CMD, 0x0000)  
        # address_vector = rom_read_word(DF_PEEK_CMD)        
        # rom_write_word(DF_POKE_CMD, 0x000A)  
        # rom_write_word(DF_ADDR_CMD, 0x0002)  
        # rom_write_word(DF_POKE_CMD, 0)  
        # rom_write_word(IF_ADDR_CMD, 0x0180)  
        
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_write_word(PG_ERASE_CMD, 0x0180)
        # time.sleep(0.5)
        
        # rom_write_word(DF_ADDR_CMD, 0x0000)  
        # address_vector = rom_read_word(DF_PEEK_CMD)         
        # rom_write_word(DF_POKE_CMD, 0x000A)  
        # rom_write_word(DF_ADDR_CMD, 0x0002)  
        # rom_write_word(DF_POKE_CMD, 0x0000)  
        
        # rom_write_word(DF_ERASE_CMD, 0x83de)
        # time.sleep(0.04)        
        # rom_write_word(IF_ERASE_CMD, 0x83de)
        # time.sleep(0.8)        
        
        '''
        ' Erase per SREC programming guide
        '''
        rom_write_word(BLK_EN_CMD, 0x83de)
        time.sleep(0.05)
        rom_write_word(0x06, 0x0)
        time.sleep(0.5)
        rom_write_word(0x07, 0x83de)
        time.sleep(0.8)
        rom_write_word(0x11, 0x83de)
        time.sleep(0.05)
        self.log.info("Erasing Ok")
        
        '''
        ' Program Like BQStudio does it
        '''
        self.log.info("Programming...")
        section1_data = bytearray([0xFF] * section1_size)
        section2_data = bytearray([0xFF] * section2_size)
        section3_data = bytearray([0xFF] * section3_size)
        
        section_info = [{'start':SECTION1_BASE, 'end':SECTION1_BASE + section1_size, 'size':section1_size, 'buffer':section1_data},
                        {'start':SECTION2_BASE, 'end':SECTION2_BASE + section2_size, 'size':section2_size, 'buffer':section2_data},
                        {'start':SECTION3_BASE, 'end':SECTION3_BASE + section3_size, 'size':section3_size, 'buffer':section3_data}]
        
        for line in testlist_array:
            try:
                data = bytes.fromhex(line[2:].strip())
            except:
                print(repr(line[2:].strip()))
                raise
        
            found = False
        
            if not(line.startswith('S2') or line.startswith('S3')):
                self.log.info("Ignoring Line that is not S2 or S3")
            elif line.startswith('S2'):
                data_length = data[0] - 4
                addr = int.from_bytes(data[1:4], 'big')
                
                for section in section_info:               
                    if section['start'] <= addr < section['end']:
                        try:
                            struct.pack_into('{}s'.format(data_length), section['buffer'], addr - section['start'], data[4:])
                            found = True
                        except:
                            print(type(section['buffer']), addr - section['start'], type(data[4:]))
                            raise
                        break
                        
            elif line.startswith('S3'):
                raise NotImplementedError()
                
            if not found:
                print(''.join(line))
            
        # Write section 1
        block_size = 32
        assert section1_size % block_size == 0
        for offset in range(0, section1_size, block_size):
            rom_write_block(DF_WRITE_CMD, struct.pack('<H{}s'.format(block_size), offset + SECTION1_BASE, section1_data[offset:(offset + block_size)]))
            time.sleep(0.005)
        time.sleep(0.04)
        rom_read_word(0x14)
            
        # Write Section 2
        block_size = 32
        assert section2_size % block_size == 0
        for offset in range(0, section2_size, block_size):
            rom_write_block(IF_WRITE_CMD, struct.pack('<H{}s'.format(block_size), (offset + SECTION2_BASE) & 0xFFFF, section2_data[offset:(offset + block_size)]))
            time.sleep(0.005)
        time.sleep(0.04)
        rom_read_word(0x12)
        
        def send_row(offset, buffer, len):        
            # rom_write_word(0, offset)
            # time.sleep(0.04)
            rom_write_word(BLK_EN_CMD, 0x83de)            
            time.sleep(0.04)
            rom_write_block(IF_WRITE_CMD, struct.pack('<H32s', offset & 0xFFFF, buffer[offset:offset+len]))
            time.sleep(0.04)
            # rom_write_word(0, offset)
            # rom_write_word(BLK_EN_CMD, 0x83de)
            # time.sleep(0.04)
            # rom_read_word(0x13)            
        
        # Section 3 (The first two data bytes of this section will be written last in Section4 and will be replaced with 0xFF while writing Section3.)
        # rom_write_word(0, 0)
        # time.sleep(0.04)
        rom_write_word(BLK_EN_CMD, 0x83de)
        time.sleep(0.04)
        rom_write_block(IF_WRITE_CMD, struct.pack('<HH30s', SECTION3_BASE & 0xFFFF, 0xFFFF, section3_data[2:32]))
        time.sleep(0.04)
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_write_word(IF_ADDR_CMD, 0)
        # rom_write_word(IF_ADDR_CMD, 0)
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_read_word(0x13)
        
        send_row(0x0020, section3_data, 32)
        send_row(0x0040, section3_data, 32)
        send_row(0x0060, section3_data, 32)
        send_row(0x0080, section3_data, 32)
        send_row(0x00A0, section3_data, 32)
        send_row(0x00C0, section3_data, 32)
        send_row(0x00E0, section3_data, 32)

        
        # Section 4
        # rom_write_word(0, 0)
        rom_write_word(BLK_EN_CMD, 0x83de)
        time.sleep(0.04)
        rom_write_block(IF_WRITE_CMD, struct.pack('<H2s', 0, section3_data[0:2]))
        time.sleep(0.04)
        
        # rom_write_word(0, 0)
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # rom_write_word(DF_ADDR_CMD, 0)    
        # address_vector = rom_read_word(DF_PEEK_CMD)
        # rom_write_word(DF_POKE_CMD, 0x00A0)
        # time.sleep(0.02)
        # rom_write_word(DF_ADDR_CMD, 0x0002)
        # rom_write_word(DF_POKE_CMD, 0)
        # time.sleep(0.02)
        
        rom_write_cmd(bytearray((0x08, 0x11)))  # Execute command is sent here 
        
        time.sleep(5)
            

class ICCSmartBattery(SmartBattery):
    def __init__(self, device_name, log, interface=None):
        super(ICCSmartBattery, self).__init__(device_name, log, interface)

        self.SMBDAT_COMMANDS.update({
            "Cell2Voltage": 0x3e,
            "Cell1Voltage": 0x3f
        })
        
        self.SMBDAT_COMMAND_TYPE.update({
            "Cell2Voltage": SmartBattery.SMB_WD | SmartBattery.SMB_PEC,
            "Cell1Voltage": SmartBattery.SMB_WD | SmartBattery.SMB_PEC
        })


import datetime
import threading

from zebra.util.ezthread import EZThread


class SmartBatteryMonitor(object):

    class SBMWatchdog(EZThread):
        def __init__(self, action, interval, log):
            EZThread.__init__(self, log=log, log_name="SBMWatchdog")
            self.ping = threading.Event()
            self.interval = interval
            self.action = action
            self.arm = False

        def pong(self):
            self.ping.set()

        def setup(self):
            self.log.info('Starting SBMWatchdog')

        def teardown(self):
            self.log.info('Finishing SBMWatchdog')

        def loop(self):
            self.ping.clear()
            if not self.ping.wait(10 * self.interval):

                if self.arm:
                    self.log.error('2nd-stage ping timeout: watchdog terminating process')
                    exit(-1)

                self.arm = True

                import collections
                if isinstance(self.action, collections.Callable):
                    self.log.error('Ping timeout (%.2f): performing action.', self.interval)
                    self.action()
                else:
                    self.log.error('Ping timeout (%.2f): no action specified.', self.interval)
            else:
                self.arm = False

    def __init__(self, device_name, log, commands, file_name=None, log_name=None, battery=None, interface=None):
        if log_name is None:
            log_name = self.__class__.__name__
        self.log = logger.get_library_logger(log_name, parent=log)

        self.device_name = device_name
        if isinstance(battery, SmartBattery):
            self._battery = battery
        else:
            self._battery = None
            self.interface = interface
        self.batteryInfo = {}
        self._command_list = commands
        self.file = file_name
        self.logger_ready = threading.Event()

        self.watchdog = None

        self.timer = None

    _POLLING_INTERVAL = 0.150

    def start(self):
        if self._battery is None:
            # TODO: Make this auto-detect
            self._battery = RolexBattery(self.device_name, self.log, self.interface)
        for key in self._command_list:
            self.batteryInfo[key] = None

        if self.file is not None:
            # Print the header
            result = "Time, "
            for key in self._command_list:
                result += "%s, " % (key,)

            with open(self.file, "ab") as fp:
                fp.write("%s\r\n" % (result,))

        self.watchdog = self.SBMWatchdog(self._start_thread, self._POLLING_INTERVAL, self.log)
        self.status_poll()
        self.watchdog.start()

    def status_poll(self):
        # MAIN LOOP
        try:
            self.loop()
            self.logger_ready.set()

        except BaseException as err:  # pylint: disable=W0703
            self.log.exception("Unhandled exception in thread loop(), terminating. %s", err)
            raise err

        finally:
            self.watchdog.pong()
            self._start_thread()

    def _start_thread(self):
        self.timer = threading.Timer(self._POLLING_INTERVAL, self.status_poll)
        self.timer.start()

    @property
    def polling_interval(self):
        return self._POLLING_INTERVAL

    @polling_interval.setter
    def polling_interval(self, val):
        if self.timer is not None:
            self.timer.cancel()

        self._POLLING_INTERVAL = val

        if self.timer is not None:
            self._start_thread()

    def loop(self):
        result = datetime.datetime.now().isoformat() + ", "

        for cmd in self._command_list:
            try:
                self.batteryInfo[cmd] = self._battery[cmd]
            except Smbdat.CommandError as e:
                self._battery.send_reset()
                self.log.error('Send Command exception occurred, command(%s) value: %x (tid:%d)',
                               cmd,
                               e.value,
                               threading.currentThread().ident)
                if self.timer is not None:
                    self.log.error('tid = %d', self.timer.ident)
                result += "ERR(%i), " % (e.value,)
            except IndexError as e:
                self.log.error('IndexError: %s', e)
            except BaseException as e:
                self.log.exception("An error has occurred trying to read battery!")
                raise e
            else:
                try:
                    if (type(self.batteryInfo[cmd]) is str) and not (cmd in SmartBattery.SMBDAT_AS_HEX):
                        result += "%s, " % (self.batteryInfo[cmd],)
                    elif type(self.batteryInfo[cmd]) is int:
                        if cmd in SmartBattery.SMBDAT_AS_HEX:
                            result += "%#04x, " % (self.batteryInfo[cmd],)
                        else:
                            result += "%i, " % (self.batteryInfo[cmd],)
                    else:
                        result += "%s, " % (":".join("{:02x}".format(ord(c)) for c in self.batteryInfo[cmd]),)
                except:
                    self.log.exception("An error has occurred trying to parse battery data!")
                    raise BaseException()

        with open(self.file, "ab") as fp:
            fp.write("%s\r\n" % (result,))

    def stop(self):
        if self.watchdog is not None:
            self.watchdog.stop()
        self.logger_ready.clear()
        if self.timer is not None:
            self.timer.cancel()


def run_example():
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

    battery = None
    try:
        battery = SmartBattery(args.batt_com, log)
        if battery['ManufacturerName'] in ('Panasonic', 'SANYO'):
            battery.interface.disconnect()
            battery = PanasonicSmartBattery(args.batt_com, log)
            print("Panasonic battery detected")
        elif battery['ManufacturerName'] in ('ICC', 'ICCN'):
            battery.interface.disconnect()
            battery = ICCSmartBattery(args.batt_com, log)
            print("ICC battery detected")
    except Smbdat.CommandError:
        pass
    finally:
        print("Unknown battery")
        if battery is None:
            battery = SmartBattery(args.batt_com, log)

    for key in battery:
        print(("%25s(%#04x) = " % (key, SmartBattery.SMBDAT_COMMANDS[key]))),
        try:
            response = battery[key]
        except Smbdat.CommandError as e:
            battery.send_reset()
            print('Send Command exception occurred, value:', e.value)
            # traceback.print_exc()
            # print("ERR(%i)" % (e.value,))
        else:
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
                print('%s%d/%d/%d' %
                      (' ' * 35, (response & 0x1F0) >> 5, response & 0x1F, ((response & 0xFE00) >> 9) + 1980))
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
                for field, val in zip(fields, data):
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
                for field, val in zip(fields, data):
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

                status = ['System failure (see 2.4.17 -> 2.4.19)(ERR_AFE, ERR_MEMORY, ERR_CHECKSUM)',
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

                status = ['Reserved',
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

if __name__ == "__main__":
    run_example()
