from .cy_smbdat import CyI2cSmbus, CY_ERROR_I2C_NAK_ERROR
from ee.equipment.smbdat import SmartBattery, mask_test
from ee.utils.crc8 import CRC8
from collections import OrderedDict
import struct
from zstruct import Struct, Type, Format, ArrayElement

class CyBQ28Z610(SmartBattery):
    def __init__(self, device_name=None, log=None, interface=None, bd_serial=None):
        if interface is None:
            self.interface = CyI2cSmbus(serial=bd_serial)
            
        else:
            assert type(interface) is CyI2cSmbus
            self.interface = interface

        super(CyBQ28Z610, self).__init__(device_name, log, interface=self.interface)

    BQ28Z610_DEVICE_ADDRESS = 0x55

    SMBDAT_COMMANDS = OrderedDict([
        ("AtRate", 0x02),
        ("AtRateTimeToEmpty", 0x04),
        ("Temperature", 0x06),
        ("Voltage", 0x08),
        ("BatteryStatus", 0x0A),
        ("Current", 0x0C),
        ("RemainingCapacity", 0x10),
        ("FullChargeCapacity", 0x12),
        ("AverageCurrent", 0x14),
        ("AverageTimeToEmpty", 0x16),
        ("AverageTimeToFull", 0x18),
        ("StandbyCurrent", 0x1A),
        ("StandbyTimeToEmpty", 0x1C),
        ("MaxLoadCurrent", 0x1E),
        ("MaxLoadTimeToEmpty", 0x20),
        ("Average Power", 0x22),
        ("InternalTemperature", 0x28),
        ("CycleCount", 0x2A),
        ("RelativeStateOfCharge", 0x2C),
        ("StateOfHealth", 0x2E),
        ("ChargeVoltage", 0x30),
        ("ChargeCurrent", 0x32),
        ("DesignCapacity", 0x3C),
        ("ModelNumber", 0xFF),
    ])

    SMBDAT_COMMAND_TYPE = OrderedDict([
        ("AtRate", SmartBattery.SMB_WD),
        ("AtRateTimeToEmpty", SmartBattery.SMB_WD),
        ("Temperature", SmartBattery.SMB_WD),
        ("Voltage", SmartBattery.SMB_WD),
        ("BatteryStatus", SmartBattery.SMB_WD),
        ("Current", SmartBattery.SMB_WD),
        ("RemainingCapacity", SmartBattery.SMB_WD),
        ("FullChargeCapacity", SmartBattery.SMB_WD),
        ("AverageCurrent", SmartBattery.SMB_WD),
        ("AverageTimeToEmpty", SmartBattery.SMB_WD),
        ("AverageTimeToFull", SmartBattery.SMB_WD),
        ("StandbyCurrent", SmartBattery.SMB_WD),
        ("StandbyTimeToEmpty", SmartBattery.SMB_WD),
        ("MaxLoadCurrent", SmartBattery.SMB_WD),
        ("MaxLoadTimeToEmpty", SmartBattery.SMB_WD),
        ("Average Power", SmartBattery.SMB_WD),
        ("InternalTemperature", SmartBattery.SMB_WD),
        ("CycleCount", SmartBattery.SMB_WD),
        ("RelativeStateOfCharge", SmartBattery.SMB_WD),
        ("StateOfHealth", SmartBattery.SMB_WD),
        ("ChargeVoltage", SmartBattery.SMB_WD),
        ("ChargeCurrent", SmartBattery.SMB_WD),
        ("DesignCapacity", SmartBattery.SMB_WD),
        ("ModelNumber", SmartBattery.SMB_BLK),
    ])

    SMBDAT_AS_HEX = (
        "BatteryStatus",
    )

    SMBDAT_IS_SIGNED = (
        "AtRate",
        "Current",
        "AverageCurrent",
        "StandbyCurrent",
        "MaxLoadCurrent",
        "AveragePower"
    )

    MFG_BLK_RD_NUM_TRIES = 50

    def read(self, cmd):
        if cmd == 'ModelNumber':
            return 'BQ28Z610'
	
        if cmd in self.SMBDAT_COMMANDS:
            retval = self.read_word(self.SMBDAT_COMMANDS[cmd])

            is_signed = self._is_signed(cmd)
            format_char = 'H'
            if is_signed:
                format_char = format_char.lower()
            else:
                format_char = format_char.upper()

            formatted_buffer = struct.unpack('<' + format_char, retval)[0]

            return formatted_buffer

    def write(self, cmd, buffer):
        if cmd in self.SMBDAT_COMMANDS:
            is_signed = self._is_signed(cmd)
            length = 2
            format_char = 'H'

            if is_signed:
                format_char = format_char.lower()
            else:
                format_char = format_char.upper()

            formatted_buffer = struct.pack('<' + format_char, buffer)

            retval = self.write_word(self.SMBDAT_COMMANDS[cmd], formatted_buffer)

            return length

    BQ28Z610_MFG_ACCESS  = 0x00
    BQ28Z610_MAC_BLOCK  = 0x3E
    BQ28Z610_MAC_DATA  = 0x40
    BQ28Z610_MAC_DATA_CHK  = 0x60
    BQ28Z610_MAC_DATA_LEN  = 0x61

    def write_word(self, cmd, buff):
        return self.interface.write_word(self.BQ28Z610_DEVICE_ADDRESS,
                                         cmd,
                                         buff,
                                         use_pec=False)

    def read_word(self, cmd):
        return self.interface.read_word(self.BQ28Z610_DEVICE_ADDRESS,
                                        cmd,
                                        use_pec=False)

    # Alt Mfg Commands
    DEVICE_TYPE = 1
    FIRMWARE_VERSION = 2
    HARDWARE_VERSION = 3
    CHEM_ID = 6
    DSG_FET_CTRL = 0x20
    GAUGE_EN = 0x21
    FET_CTRL = 0x22
    LIFETIME_DATA = 0x23
    PF_EN = 0x24
    SEAL_DEVICE = 0x30
    DEVICE_RESET = 0x41
    SAFETY_STATUS = 0x51
    PF_STATUS = 0x53
    OPERATION_STATUS = 0x54
    CHARGING_STATUS = 0x55
    GAUGING_STATUS = 0x56
    MANUFACTURING_STATUS = 0x57
    DA_STATUS1 = 0x0071

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
    
    def mfg_blk_rd(self, alt_cmd):
        # Select Alt Mfg Register
        cmd_buff = struct.pack('<H', alt_cmd)
        retry_count = 5

        mac_blk = None
        chksum = None

        while retry_count:
            try:
                self.write_word(self.BQ28Z610_MFG_ACCESS, cmd_buff)

                # Get Alt Mfg Data CRC and Len
                chksum, length = self.read_word(self.BQ28Z610_MAC_DATA_CHK)

                # Read out Alt Mfg Data (plus command to verify)
                self.interface.write_bytearray(self.BQ28Z610_DEVICE_ADDRESS, bytearray([self.BQ28Z610_MAC_BLOCK]))
                mac_blk = self.interface.read_bytearray(self.BQ28Z610_DEVICE_ADDRESS, length - 2)

                # print((chksum, length))
                # print("%s" % (":".join("{:02x}".format(c) for c in mac_blk),))
                
                command_rx = struct.unpack('<H', mac_blk[:2])[0]
                if (alt_cmd != command_rx):
                    raise ValueError('Alt command verify failed got({:02x}) expected({:02x})'.format(command_rx, alt_cmd))
                if chksum != (~sum(mac_blk) & 0xFF):
                    raise ValueError('chksum failed.')
                
            except (CY_ERROR_I2C_NAK_ERROR, IOError, ValueError):
                if retry_count == 0:
                    raise
                    
                if self.log:
                    self.log.debug('Exception occured', exc_info=True)
                retry_count -= 1
                time.sleep(0.1)
            else:
                break
                
        return mac_blk[2:]

    def alt_mfg_cmd(self, alt_cmd, data=None):
        retval = CyI2cSmbus.CY_SUCCESS
        retry_count = 5
        
        while retry_count:
            try:
             
                if data:
                    cmd_buff = bytearray(struct.pack('<BH', self.BQ28Z610_MAC_BLOCK, alt_cmd))
                    cmd_buff.extend(data)
                else:
                    cmd_buff = bytearray(struct.pack('<BH', self.BQ28Z610_MFG_ACCESS, alt_cmd))

                chksum = (~sum(cmd_buff[1:]) & 0xFF)
                length = len(cmd_buff) + 1

                # print("%s" % (":".join("{:02x}".format(c) for c in cmd_buff),))

                if self.interface.write_bytearray(self.BQ28Z610_DEVICE_ADDRESS, cmd_buff) != CyI2cSmbus.CY_SUCCESS:
                    raise IOError('alt_mfg_cmd Select Failed')
                if data:
                    cmd_buff = bytearray(struct.pack('<BBB', self.BQ28Z610_MAC_DATA_CHK, chksum, length))

                    # print("%s" % (":".join("{:02x}".format(c) for c in cmd_buff),))

                    retval = self.interface.write_bytearray(self.BQ28Z610_DEVICE_ADDRESS, cmd_buff)

                if retval != CyI2cSmbus.CY_SUCCESS:
                    raise IOError('alt_mfg_cmd failed to complte')

            except (CY_ERROR_I2C_NAK_ERROR, IOError, ValueError):
                if retry_count == 0:
                    raise

                if self.log:
                    self.log.debug('Exception occured', exc_info=True)
                retry_count -= 1
                time.sleep(0.1)
            else:
                break

    def get_firmware_info(self):
        device_number, version, build_number, fw_type, it_ver, rsvd = struct.unpack('>HHHHHB', self.mfg_blk_rd(self.FIRMWARE_VERSION))
        return (device_number, version, build_number, fw_type, it_ver, rsvd)

    # Data Flash Addresses
    UPDATE_STATUS = 0x420E
    TERM_VOLTAGE = 0x45BE
    SOH_LOAD_RATE = 0x45E8
    FET_OPTIONS = 0x4600
    SOC_FLAG_CONFIG_A = 0x4632
    SOC_FLAG_CONFIG_B = 0x4634
    CHARGE_TERM_TAPER_CURRENT = 0x4693
    CHARGE_TERM_VOLTAGE = 0x4697
    DSCRHG_CURRENT_THRESH = 0x46A6
    CHRGE_CURRENT_THRESH = 0x46A8
    QUIT_CURRENT_THRESH = 0x46AA
    SBS_GAUGING_CONF = 0x4601
    SHUTDOWN_VALUE = 0x460C
    IT_GAUGING_CONF = 0x464D
    CUV_THRESH = 0x46B3

    def df_rd(self, mem_addr, size=0x20):
        retry_count = 5

        size = min(size, 0x20)

        while retry_count:
            try:
                if not(0x4000 <= mem_addr <= 0x5FFF):
                    raise IndexError('DF address out of bounds')
                # Select Alt Mfg Register
                cmd_buff = struct.pack('<H', mem_addr)
                self.write_word(self.BQ28Z610_MAC_BLOCK, cmd_buff)

                # Read out Alt Mfg Data (plus command to verify)
                self.interface.write_bytearray(self.BQ28Z610_DEVICE_ADDRESS, bytearray([self.BQ28Z610_MAC_BLOCK]))
                mac_blk = self.interface.read_bytearray(self.BQ28Z610_DEVICE_ADDRESS, size + 2)

                # print("%s" % (":".join("{:02x}".format(c) for c in mac_blk),))

                if mem_addr != struct.unpack('<H', mac_blk[:2])[0]:
                    raise ValueError('mem_addr mismatch')

                return mac_blk[2:]
            except (CY_ERROR_I2C_NAK_ERROR, IOError, ValueError):
                if retry_count == 0:
                    raise
                if self.log:
                    self.log.debug('Exception occured', exc_info=True)
                retry_count -= 1
                time.sleep(0.1)
            else:
                break

    def df_wd_rd(self, mem_addr):
        return struct.unpack('<H', self.df_rd(mem_addr, 2)[:2])[0]

    def df_byte_rd(self, mem_addr):
        return struct.unpack('<B', self.df_rd(mem_addr, 1)[:1])[0]

    def df_wr(self, mem_addr, data):
        retry_count = 5

        while retry_count:

            try:
                if not(0x4000 <= mem_addr <= 0x5FFF):
                    raise IndexError('DF address out of bounds')
                try:
                    self.alt_mfg_cmd(mem_addr, data)
                except CY_ERROR_I2C_NAK_ERROR:
                    pass

            except (CY_ERROR_I2C_NAK_ERROR, IOError, ValueError):
                if retry_count == 0:
                    raise

                if self.log:
                    self.log.debug('Exception occured', exc_info=True)
                retry_count -= 1
                time.sleep(0.1)
            else:
                break

        time.sleep(0.1)

    IT_STAT_OCVFR = 0x1000
    IT_STAT_LDMD = 0x0800
    IT_STAT_RX = 0x0400
    IT_STAT_QMAX = 0x0200
    IT_STAT_VDQ = 0x0100
    IT_STAT_NSFM = 0x0080
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
    
    IT_STATUS_FLAGS = {'OCVFR': 0x1000,
                       'LDMD': 0x0800,
                       'RX': 0x0400,
                       'QMAX': 0x0200,
                       'VDQ': 0x0100,
                       'NSFM': 0x0080,
                       'SLPQ': 0x0020,
                       'QEN': 0x0010,
                       'VOK': 0x0008,
                       'RDIS': 0x0004,
                       'REST': 0x0001}
    
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

    @property
    def da_stat1(self):
        return self.DaStat1.read_pack(self)
        
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
        op_stat = struct.unpack('<L', bytes(self.mfg_blk_rd(self.OPERATION_STATUS)))[0]
        return (op_stat & 0x300) >> 8


    def device_reset(self):
        self.alt_mfg_cmd(self.DEVICE_RESET)
        self.send_reset()
        time.sleep(5)
        self.mfg_blk_rd(self.DEVICE_TYPE)

    SECURITY_MODE_SEALED = 3
    SECURITY_MODE_UNSEAL = 2
    SECURITY_MODE_FULL = 1
    DEFAULT_UNSEAL_KEY = [0x0414, 0x3672]
    DEFAULT_FULL_ACCESS_KEY = [0xFFFF, 0xFFFF]

    def unseal_device(self, key=(0x0414, 0x3672)):
        self.alt_mfg_cmd(key[0])
        self.alt_mfg_cmd(key[1])
        if (self.security_mode() != self.SECURITY_MODE_UNSEAL):
            raise ValueError('Failed to unseal pack')

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
        # for name, mask in flag_dict.items():
            # print(F'{field_reg:x} {mask:x} {(field_reg & mask):x} {((field_reg & mask) != 0)}')
    
        # print(F'{flag_dict!r} {field_reg!r}')
        return [name for name, mask in flag_dict.items() if ((field_reg & mask) != 0)]
    
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

                while address <= end_addr:
                    response = self.df_rd(address, size=min((end_addr - address + 1), 32))
                    flash_data_buffer.extend(response[:min(len(response), end_addr - address + 1)])
                    address += len(response)

            except (AssertionError, ValueError, IOError) as exc:
                self.log.error(exc)
                self.send_reset()
                time.sleep(0.5)
                if i == (self.MFG_BLK_RD_NUM_TRIES - 1):
                    raise exc
            else:
                break

        # self.log.debug('read_flash_data() returned %d bytes', len(flash_data_buffer))

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
        rsvd_6 = ArrayElement(Type.UnsignedByte, 0x10)
        fast_scale_start = Type.UnsignedByte
        rsvd_7 = ArrayElement(Type.UnsignedByte, 8)
        delta_voltage_min = Type.Short
        rsvd_8 = ArrayElement(Type.UnsignedByte, 0x69)
        load_select = Type.UnsignedByte
        load_mode = Type.UnsignedByte
        user_rate_ma = Type.Short
        user_rate_cw = Type.Short
        reserve_cap_ma = Type.Short
        reserve_cap_cw = Type.Short

        @classmethod
        def read_pack(cls, pack):
            try:
                buffer = pack.read_flash_data(0x4200, 0x465A)
                return cls(buffer)            
            except struct.error:
                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
                raise
    @property
    def df_it_cfg(self):
        return self.DataFlashITCfg.read_pack(self) 
            
    class DataFlashGasGauge(Struct):
        _format = Format.LittleEndian
        qmax_cell_1 = Type.Short
        qmax_cell_2 = Type.Short
        qmax_pack = Type.Short
        qmax_cycle_count = Type.UnsignedShort
        update_status = Type.UnsignedByte
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
        rsvd_1 = ArrayElement(Type.UnsignedByte, 29)
        cycle_count = Type.UnsignedShort
        rsvd_2 = ArrayElement(Type.UnsignedByte, 1251)
        soh_temp_k = Type.Short
        soh_temp_a = Type.Short        

        @classmethod
        def read_pack(cls, pack):
            try:
                buffer = pack.read_flash_data(0x4206, 0x4241)
                buffer.extend([0xFF] * 1251)
                buffer.extend(pack.read_flash_data(0x4725, 0x4728))
                return cls(buffer)            
            except struct.error:
                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
                raise
            
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
            return cls(pack.read_flash_data(0x4280, 0x4289))

    @property
    def df_ra_table(self):
        return self.DataFlashRa.read_pack(self) 
            
    class DataFlashRa(Struct):
        _format = Format.LittleEndian
        cell_0_R_a_flag = Type.UnsignedShort
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
        cell_1_R_a_flag = Type.UnsignedShort
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
        xcell_0_R_a_flag = Type.UnsignedShort
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
        xcell_1_R_a_flag = Type.UnsignedShort
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
            buffer_data = pack.read_flash_data(0x4100, 0x411F)
            buffer_data.extend(pack.read_flash_data(0x4140, 0x415F))
            buffer_data.extend(pack.read_flash_data(0x4180, 0x419F))
            buffer_data.extend(pack.read_flash_data(0x41C0, 0x41DF))
            return cls(buffer_data)

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
                                               use_pec=False)
            return struct.unpack("<H", retval)[0]

        def rom_write_word(command_code, data):
            # print('WW %#04x %#06x'%(command_code, data))
            # return
            retval =  self.interface.write_word(ROM_DEVICE_ADDRESS, command_code, struct.pack("<H", data), use_pec=False)
            time.sleep(0.001)

        def rom_write_cmd(command_code):
            # print('WC %#04x'%(command_code,))
            # return
            return self.interface.write_bytearray(ROM_DEVICE_ADDRESS, bytearray([command_code]))

        section4_data = [0] * 4
        SECTION1_BASE = 0x4000
        SECTION2_BASE = 0x100000
        SECTION3_BASE = 0x140000
        section1_size = 0x0860 # 8*1024
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
        DF_WRITE_CMD = 0x0f
        DF_ERASE_CMD = 0x11
        BLK_EN_CMD = 0x1A
        data_array_index = 0

        with open(srec) as f:
            testlist_array = list(f)

        # Jump to ROM loader
        try:
            self.write_word(0, struct.pack("<H", 0x0033))
        except CY_ERROR_I2C_NAK_ERROR:
            pass

        time.sleep(0.1)
        rom_version = rom_read_word(0x0D)
        self.log.info('Read word 0x0D : %#04X', rom_version)

        assert rom_version == 0x9101, "Part is incompatible with programming"

        self.log.info("Erasing...")
        '''
        ' BQStudio does this and so will we
        '''
        rom_write_word(DF_ADDR_CMD, 0)
        rom_write_word(DF_POKE_CMD, 0)
        rom_write_word(DF_ADDR_CMD, 0x0002)
        rom_write_word(DF_POKE_CMD, 0)
        rom_write_word(IF_ADDR_CMD, 0)
        rom_write_word(BLK_EN_CMD, 0x83de)
        rom_write_word(PG_ERASE_CMD, 0)
        time.sleep(0.5)
        rom_write_word(IF_ADDR_CMD, 0x0080)
        rom_write_word(BLK_EN_CMD, 0x83de)
        rom_write_word(PG_ERASE_CMD, 0x0080)
        time.sleep(0.5)
        rom_write_word(DF_ADDR_CMD, 0x0000)    
        rom_write_word(DF_POKE_CMD, 0x0008)  
        rom_write_word(DF_ADDR_CMD, 0x0002)  
        rom_write_word(DF_POKE_CMD, 0x00B8)  
        rom_write_word(IF_ADDR_CMD, 0x0180)  
        rom_write_word(BLK_EN_CMD, 0x83de)
        rom_write_word(PG_ERASE_CMD, 0x0180)
        time.sleep(0.5)
        rom_write_word(DF_ADDR_CMD, 0x0000)    
        rom_write_word(DF_POKE_CMD, 0x0000)
        rom_write_word(DF_ADDR_CMD, 0x0002)
        rom_write_word(DF_POKE_CMD, 0x0000)
        rom_write_word(DF_ERASE_CMD, 0x83de)
        time.sleep(0.04)
        rom_write_word(IF_ERASE_CMD, 0x83de)
        time.sleep(0.8)
        '''
        ' Erase per SREC programming guide
        '''
        # rom_write_word(BLK_EN_CMD, 0x83de)
        # time.sleep(0.05)
        # rom_write_word(0x06, 0x0)
        # time.sleep(0.05)
        # rom_write_word(0x07, 0x83de)
        # time.sleep(0.05)
        # rom_write_word(0x11, 0x83de)
        # time.sleep(0.05)
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
        
            if not(line.startswith('S2') or line.startswith('S3')):
                self.log.info("Ignoring Line that is not S2 or S3")
            elif line.startswith('S2'):
                data_length = data[0] - 4
                addr = int.from_bytes(data[1:4], 'big')
                
                for section in section_info:               
                    if section['start'] <= addr < section['end']:
                        try:
                            struct.pack_into('{}s'.format(data_length), section['buffer'], addr - section['start'], data[4:])
                        except:
                            print(type(section['buffer']), addr - section['start'], type(data[4:]))
                            raise
                        break
            elif line.startswith('S3'):
                raise NotImplementedError()
            
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
            rom_write_word(0, offset)
            time.sleep(0.04)
            rom_write_word(BLK_EN_CMD, 0x83de)            
            rom_write_block(IF_WRITE_CMD, struct.pack('<H32s', offset & 0xFFFF, buffer[offset:offset+len]))
            time.sleep(0.04)
            rom_write_word(0, offset)
            time.sleep(0.04)
            rom_read_word(0x13)
        
        
        send_row(0x0020, section3_data, 32)
        send_row(0x0040, section3_data, 32)
        send_row(0x0060, section3_data, 32)
        
        # Section 3 (The first two data bytes of this section will be written last in Section4 and will be replaced with 0xFF while writing Section3.)
        rom_write_word(0, 0)
        time.sleep(0.04)
        rom_write_word(BLK_EN_CMD, 0x83de)
        rom_write_block(IF_WRITE_CMD, struct.pack('<HH30s', SECTION3_BASE & 0xFFFF, 0xFFFF, section3_data[2:32]))
        time.sleep(0.04)
        rom_read_word(0x13)
        
        send_row(0x0080, section3_data, 32)
        send_row(0x00A0, section3_data, 32)
        send_row(0x00C0, section3_data, 32)
        send_row(0x00E0, section3_data, 32)
        
        # Section 4
        rom_write_word(0, 0)
        rom_write_word(BLK_EN_CMD, 0x83de)
        rom_write_block(IF_WRITE_CMD, struct.pack('<H2s', 0, section3_data[0:2]))
        time.sleep(0.04)
        
        rom_write_cmd(0x08)  # Execute command is sent here
        
        
        '''
        self.log.info("Programming...")
        section_addr = 0
        for testlist_index in range(0, len(testlist_array), 1):
            ful_row = testlist_array[testlist_index]
            if (ful_row[0:2] != 'S2') and (ful_row[0:2] != 'S3'):
                # discard non S2 and S3 rows i.e. Non-data rows
                self.log.info("Ignoring Line that is not S2 or S3")
            elif ful_row[0:2] == 'S2':  # If srec file is using S2 format data (24-bit address)
                data_length = int(ful_row[2:4], 16)
                addr = int(ful_row[4:10], 16)
                # Check for Section1 data first, compile the data starting at
                # 0x4000 and ending at 0x5FFF into 64 bit lists
                if (addr >= SECTION1_BASE) and (addr <= (SECTION1_BASE + section1_size)):
                    if addr == SECTION1_BASE:
                        section_addr = addr  # Compile the address and data for block write to the device in a list
                        # First two items in the list will be the address bytes the first list item being the lower
                        # byte of the address
                        section1_data.append(section_addr & 0xFF)
                        section1_data.append(section_addr >> 8)  # upper byte of the address
                    # Check if all 64 items for the block write have been compiled
                    elif (section_addr % 32) == 0:
                        # Write the data to the device with command 0xF for the first section
                        rom_write_block(DF_WRITE_CMD, section1_data)
                        time.sleep(0.01)
                        section1_data = bytearray()
                        section1_data.append(section_addr & 0xFF)
                        section1_data.append(section_addr >> 8)  # upper byte of the address
                    for index in range(10, len(ful_row)-3, 2):
                        section1_data.append(int(ful_row[index:index+2], 16))
                        section_addr += 1
                    if (section_addr < (SECTION1_BASE + section1_size)) and (data_length < 0x14):
                        # Fill any Section1 address that is not present in the srec with 0xFF
                        while (section_addr % 32) != 0:
                            section1_data.append(0xFF)
                            section_addr += 1
                        section_addr -= 1
                        while section_addr < (SECTION1_BASE + section1_size):
                            if (section_addr % 32) == 0:
                                rom_write_block(DF_WRITE_CMD, section1_data)
                                time.sleep(0.01)
                                section1_data = bytearray()
                                section1_data.append(section_addr & 0xFF)
                                section1_data.append(section_addr >> 8)  # upper byte of the address
                                section1_data.append(0xFF)
                                section_addr += 1
                            else:
                                section1_data.append(0xFF)
                                section_addr += 1

                elif (addr >= SECTION2_BASE) and (addr <= (SECTION2_BASE + section2_size)):
                    if addr == SECTION2_BASE:
                        # Compile the Section2 data and address in a similar manner to Section1
                        section_addr = addr & 0xFFFF
                        section2_data.append(section_addr % 0xFF)
                        section2_data.append(section_addr >> 8)
                    elif (section_addr % 64) == 0:
                        # Write the compiled list for Section2 with command 0x5
                        rom_write_block(IF_WRITE_CMD, section2_data)
                        time.sleep(0.01)
                        section2_data = bytearray()
                        section2_data.append(section_addr % 0xFF)
                        section2_data.append(section_addr >> 8)
                    for index in range(10, len(ful_row)-3, 2):
                        section2_data.append(int(ful_row[index:index+2], 16))
                        section_addr += 1
                    if (section_addr < section2_size) and (data_length < 0x14):
                        while (section_addr % 64) != 0:
                            section2_data.append(0xFF)
                            section_addr += 1
                        rom_write_block(IF_WRITE_CMD, section2_data)
                        time.sleep(0.01)

                elif (addr >= SECTION3_BASE) and (addr <= (SECTION3_BASE + section3_size)):
                    section_addr = addr & 0x1FF
                    if section_addr == 0:
                        section3_data[0] = 0
                        section3_data[1] = 0
                        # The first two data bytes of Section3 will be replaced by 0xFF
                        section3_data[2] = 255
                        section3_data[3] = 255
                        # The first two data bytes of Section3 will be written in Section4
                        section4_data[2] = int(ful_row[10:12], 16)
                        section4_data[3] = int(ful_row[12:14], 16)
                        section_addr += 2
                        for i in range(14, len(ful_row)-3, 2):
                            section3_data[section_addr+2] = int(ful_row[i:i+2], 16)
                            section_addr += 1
                    # Section3 is written in 32 byte blocks
                    elif section_addr < 0x20:
                        for i in range(10, len(ful_row)-3, 2):
                            section3_data[(i >> 1)+13] = int(ful_row[i:i+2], 16)
                        # Section3 can only be written after writing the Block Enable key 0xDE83 with command 0x1a
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        # Section3 is also written with command 0x5
                        rom_write_block(IF_WRITE_CMD, section3_data)
                        time.sleep(0.01)
                    elif section_addr == 0x80:
                        data_array_index = 0
                        section3_data[data_array_index] = int(ful_row[8:10], 16)
                        data_array_index += 1
                        section3_data[data_array_index] = int(ful_row[6:8], 16)
                        data_array_index += 1
                        for index in range(10, len(ful_row)-3, 2):
                            section3_data[data_array_index] = int(ful_row[index:index+2], 16)
                            data_array_index += 1
                    elif section_addr == 0x90:
                        for index in range(10, len(ful_row)-3, 2):
                            section3_data[data_array_index] = int(ful_row[index:index+2], 16)
                            data_array_index += 1
                        for index in range(len(ful_row)-3, 34, 1):
                            section3_data[index] = 255
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        rom_write_block(IF_WRITE_CMD, section3_data)
                        time.sleep(0.01)
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        # The first two bytes from Section3 are written here with command 0x05 and address 0x0000
                        rom_write_block(IF_WRITE_CMD, section4_data)
                        time.sleep(0.05)
                        rom_write_cmd(0x08)  # Execute command is sent here
                        time.sleep(1)
                        testlist_index += 1
                    else:
                        pass

                else:
                    raise ValueError('Error in Address range')

            # If srec file is using S3 format data (32-bit address)
            elif ful_row[0:2] == 'S3':
                data_length = int(ful_row[2:4], 16)
                addr = int(ful_row[6:12], 16)
                # Check for Section1 data first, compile the data starting at 0x4000 and ending at 0x5FFF into
                # 64 bit lists
                if (addr >= SECTION1_BASE) and (addr <= (SECTION1_BASE + section1_size)):
                    if addr == SECTION1_BASE:
                        section_addr = addr
                        section1_data.append(section_addr % 256)
                        section1_data.append(section_addr / 256)
                    elif (section_addr % 64) == 0:
                        rom_write_block(DF_WRITE_CMD, section1_data)
                        time.sleep(0.01)
                        section1_data = []
                        section1_data.append(section_addr % 256)
                        section1_data.append(section_addr / 256)
                    # Compile the address and data for block write to the device in a list
                    for index in range(12, len(ful_row)-3, 2):
                        # First two items in the list will be the address bytes the first list item
                        # being the lower byte of the address
                        section1_data.append(int(ful_row[index:index+2], 16))
                        section_addr += 1  # Second item on the list will be the upper byte of the address
                        if (section_addr % 64) == 0:  # Check if all 64 items for the block write have been compiled
                            # Write the data to the device with command 0xF for the first section
                            rom_write_block(DF_WRITE_CMD, section1_data)
                            time.sleep(0.01)
                            section1_data = []
                            section1_data.append(section_addr % 256)
                            section1_data.append(section_addr / 256)
                    if (section_addr < (SECTION1_BASE + section1_size)) and (data_length < 0x28):
                        while (section_addr % 64) != 0:
                            section1_data.append(0xFF)
                            section_addr += 1
                        section_addr -= 1

                        # Fill any Section1 address that is not present in the srec with 0xFF
                        while section_addr < (SECTION1_BASE + section1_size):
                            if (section_addr % 64) == 0:
                                rom_write_block(DF_WRITE_CMD, section1_data)
                                time.sleep(0.01)
                                section1_data = []
                                section1_data.append(section_addr % 256)
                                section1_data.append(section_addr / 256)
                                section1_data.append(0xFF)
                                section_addr += 1
                            else:
                                section1_data.append(0xFF)
                                section_addr += 1

                # Compile the Section2 data and address in a similar manner to Section1
                elif (addr >= SECTION2_BASE) and (addr <= (SECTION2_BASE + section2_size)):
                    if addr == SECTION2_BASE:
                        section_addr = addr & 0xFFFF
                        section2_data.append(section_addr % 256)
                        section2_data.append(section_addr / 256)
                    elif (section_addr % 64) == 0:
                        rom_write_block(IF_WRITE_CMD, section2_data)
                        time.sleep(0.01)
                        section2_data = []
                        section2_data.append(section_addr % 256)
                        section2_data.append(section_addr / 256)
                    for index in range(12, len(ful_row)-3, 2):
                        section2_data.append(int(ful_row[index:index+2], 16))
                        section_addr += 1
                        if (section_addr % 64) == 0:
                            # Write the compiled list for Section2 with command 0x5
                            rom_write_block(IF_WRITE_CMD, section2_data)
                            time.sleep(0.01)
                            section2_data = []
                            section2_data.append(section_addr % 256)
                            section2_data.append(section_addr / 256)
                    if (section_addr < section2_size) and (data_length < 0x28):
                        while (section_addr % 64) != 0:
                            section2_data.append(0xFF)
                            section_addr += 1
                        rom_write_block(IF_WRITE_CMD, section2_data)
                        time.sleep(0.01)

                elif (addr >= SECTION3_BASE) and (addr <= (SECTION3_BASE + section3_size)):
                    section_addr = addr & 0x1FF
                    if section_addr == 0:
                        section3_data[0] = 0
                        section3_data[1] = 0
                        # The first two data bytes of Section3 will be replaced by 0xFF
                        section3_data[2] = 255
                        section3_data[3] = 255
                        # The first two data bytes of Section3 will be written in Section4
                        section4_data[2] = int(ful_row[12:14], 16)
                        section4_data[3] = int(ful_row[14:16], 16)
                        section_addr += 2
                        for i in range(16, len(ful_row)-3, 2):
                            if section_addr < 32:  # Section3 is written in 32 byte blocks
                                section3_data[section_addr+2] = int(ful_row[i:i+2], 16)
                            section_addr += 1
                        # Section3 can only be written after writing the Block Enable key 0xDE83 with command 0x1a
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        rom_write_block(IF_WRITE_CMD, section3_data)  # Section3 is also written with command 0x5
                        time.sleep(0.01)

                    elif section_addr == 0x69:
                        data_array_index = 0
                        section3_data[data_array_index] = 0x80
                        data_array_index += 1
                        section3_data[data_array_index] = 0x00
                        data_array_index += 1
                        for index in range(58, len(ful_row)-3, 2):
                            section3_data[data_array_index] = int(ful_row[index:index+2], 16)
                            data_array_index += 1
                    elif section_addr == 0x8c:
                        for index in range(12, 51, 2):
                            section3_data[data_array_index] = int(ful_row[index:index+2], 16)
                            data_array_index += 1
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        rom_write_block(IF_WRITE_CMD, section3_data)
                        time.sleep(0.01)
                        rom_write_word(BLK_EN_CMD, 0x83de)
                        time.sleep(0.05)
                        # The first two bytes from Section3 are written here with command 0x05 and address 0x0000
                        rom_write_block(IF_WRITE_CMD, section4_data)
                        time.sleep(0.05)
                        rom_write_cmd(0x08)  # Execute command is sent here
                        time.sleep(1)
                    else:
                        pass
                else:
                    self.log.error('Error in Address range')
                    raise ValueError('Error in Address range')
            else:
                self.log.error('Error in SREC file')
                raise ValueError('Error in SREC file')
        '''

import argparse, os, time
from zebra.util import logger
from cy7c65211 import CyUSBSerial
from cy7c65211 import CyI2C
from cy7c65211 import CyGPIO

dll = os.path.join(os.path.dirname(__file__), '../../cy7c65211/cyusbserial.dll')
battery = None
log = None


def std_cmd_rd(cmd):
    global battery, log
    
    response = None
    for i in range(battery.MFG_BLK_RD_NUM_TRIES):
        try:
            response = battery[cmd]
        except (ValueError, IOError) as e:
            log.error(e)
            time.sleep(0.5)
            if i == (battery.MFG_BLK_RD_NUM_TRIES - 1):
                raise e
        else:
            break

    return response

log_file_prefix = 'test'
ram_log_csv_name = None
df_log_csv_name = None


def log_flash_data(batt):
    gg = batt.DataFlashGasGauge.read_pack(batt)
    lt = batt.DataFlashLifetimes.read_pack(batt)
    ra = batt.DataFlashRa.read_pack(batt)

    global df_log_csv_name
    if df_log_csv_name is None:
        df_log_csv_name = log_file_prefix + '_df_log.csv'
        record_buffer = 'time, '

        for cmd in gg._struct_info:
            record_buffer += '%s, ' % (cmd[0],)
        for cmd in lt._struct_info:
            record_buffer += '%s, ' % (cmd[0],)
        for cmd in ra._struct_info:
            record_buffer += '%s, ' % (cmd[0],)

        record_buffer += '\n'
        with open(df_log_csv_name, 'w') as f:
            f.write(record_buffer)

    record_buffer = '%s, ' % (time.time(),)
    for cmd in gg._struct_info:
        record_buffer += '0x%04x, ' % (getattr(gg, cmd[0]),)
    for cmd in lt._struct_info:
        record_buffer += '0x%04x, ' % (getattr(lt, cmd[0]),)
    for cmd in ra._struct_info:
        record_buffer += '0x%04x, ' % (getattr(ra, cmd[0]),)
    record_buffer += '\n'
    with open(df_log_csv_name, 'a') as f:
        f.write(record_buffer)


def log_ram_data(batt):
    global ram_log_csv_name
    if ram_log_csv_name is None:
        ram_log_csv_name = log_file_prefix + '_ram_log.csv'
        record_buffer = 'time, '
        for cmd in batt.SMBDAT_COMMANDS.keys():
            record_buffer += '%s, ' % (cmd,)
        record_buffer += '\n'
        with open(ram_log_csv_name, 'w') as f:
            f.write(record_buffer)

    record_buffer = '%s, ' % (time.time(),)
    for cmd in batt.SMBDAT_COMMANDS.keys():
        if batt.SMBDAT_COMMAND_TYPE[cmd] & batt.SMB_WD:
            val = std_cmd_rd(cmd)
            if cmd not in batt.SMBDAT_AS_HEX:
                record_buffer += '%d, ' % (std_cmd_rd(cmd),)
            else:
                record_buffer += '0x%04x, ' % (std_cmd_rd(cmd),)
        else:
            record_buffer += "%s, " % (":".join("{:02x}".format(ord(c)) for c in std_cmd_rd(cmd)),)
    record_buffer += '\n'
    with open(ram_log_csv_name, 'a') as f:
        f.write(record_buffer)


LOG_INTERVAL_DF = 5 * 60
LOG_INTERVAL_RAM = 4


def handle_data_logs(batt):
    global last_log_time_df
    global last_log_time_ram

    now = time.time()

    if last_log_time_df is None or (now - last_log_time_df) > LOG_INTERVAL_DF:
        log_flash_data(batt)
        last_log_time_df = now

    if last_log_time_ram is None or (now - last_log_time_ram) > LOG_INTERVAL_RAM:
        log_ram_data(batt)
        last_log_time_ram = now


last_log_time_ram = None
last_log_time_df = None


def run_example():
    global battery, log
    
    parser = argparse.ArgumentParser(
        description='A smart battery data test',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument("--log_level",
                        type=int,
                        default=logger.INFO,
                        )
    args = parser.parse_args()

    log = logger.get_logger("test", level=args.log_level)

    lib = CyUSBSerial(lib = dll)
    dev = lib.find(vid=0x04B4, pid=0x0007).next()
    dev.raise_on_error = False

    i2c_dev = CyI2C(dev)
    i2c_dev.debug = False
    smbus_interface = CyI2cSmbus(device=i2c_dev)

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

    battery = CyBQ28Z610(interface=smbus_interface)

    for key in battery:
        print("%25s(%#04x) = " % (key, battery.SMBDAT_COMMANDS[key])),
        try:
            response = battery[key]
        except ValueError as e:
            battery.send_reset()
            print('Send Command exception occurred, value:', repr(e))
            # traceback.print_exc()
            # print("ERR(%i)" % (e.value,))
        else:
            try:
                if (type(response) is str) and not (key in battery.SMBDAT_AS_HEX):
                    print("%s" % (response,))
                elif type(response) is int:
                    if key in battery.SMBDAT_AS_HEX:
                        print("%#04x" % (response,))
                    else:
                        print("%i" % (response,))
                else:
                    print("%s" % (":".join("{:02x}".format(ord(c)) for c in response),))

            except struct.error as e:
                import sys
                sys.stdout.flush()
                log.exception("<failed to unpack>")
                sys.stderr.flush()
                time.sleep(.1)

    battery.unseal_device()
    print('*' * 40)
    print('0x{:02x}'.format(struct.unpack('<H', battery.mfg_blk_rd(battery.MANUFACTURING_STATUS))[0]))
    print(battery.manufacturing_status()['lifetime'])
    battery.alt_mfg_cmd(battery.LIFETIME_DATA, None)
    print('0x{:02x}'.format(struct.unpack('<H', battery.mfg_blk_rd(battery.MANUFACTURING_STATUS))[0]))
    print(battery.manufacturing_status()['lifetime'])
    while not battery.manufacturing_status()['lifetime']:
        battery.alt_mfg_cmd(battery.LIFETIME_DATA, None)
        print('0x{:02x}'.format(struct.unpack('<H', battery.mfg_blk_rd(battery.MANUFACTURING_STATUS))[0]))
        print(battery.manufacturing_status()['lifetime'])
    print('*' * 40)

    log.info('Battery voltage: %d mV', std_cmd_rd("Voltage"))
    battery.df_wr(battery.SOC_FLAG_CONFIG_A, bytearray((0xFC, 0x0C)))   # SOC Flag Config A (FC works with C8C)
    battery.df_wr(battery.SOC_FLAG_CONFIG_B, bytearray((0x8C,)))   # SOC Flag Config B (FC works with 0C)
    battery.df_wr(battery.UPDATE_STATUS, bytearray((0x04,)))   # Update Status

    battery.fet_ctrl(True)

    log.info('Battery voltage: %d mV', std_cmd_rd("Voltage"))
    log.info('Device Type: %s', (":".join("{:02x}".format(ord(c)) for c in battery.mfg_blk_rd(battery.DEVICE_TYPE))))
    log.info('Firmware Version: %s', (":".join("{:02x}".format(ord(c)) for c in battery.mfg_blk_rd(battery.FIRMWARE_VERSION))))
    log.info('Hardware Version: %s', (":".join("{:02x}".format(ord(c)) for c in battery.mfg_blk_rd(battery.HARDWARE_VERSION))))
    log.info('-')
    log.info('2.0 Chem ID: 0x%04x', struct.unpack('<H', battery.mfg_blk_rd(battery.CHEM_ID))[0])
    design_capacity = std_cmd_rd('DesignCapacity')
    log.info('3.1 Design Capacity: %d', design_capacity)

    battery.unseal_device()
    log.info('Device is unsealed.')
    
    term_taper_i_ma = struct.unpack('<h', battery.df_rd(battery.CHARGE_TERM_TAPER_CURRENT)[:2])[0]
    log.info('3.2 Charge Termination Taper Current: %d mA', term_taper_i_ma)
    log.info('3.3 Discharge Current Threshold: %d mA', struct.unpack('<h', battery.df_rd(battery.DSCRHG_CURRENT_THRESH)[:2])[0])
    log.info('3.4 Charge Current Threshold: %d mA', struct.unpack('<h', battery.df_rd(battery.CHRGE_CURRENT_THRESH)[:2])[0])
    log.info('3.5 Quit Current Threshold: %d mA', struct.unpack('<h', battery.df_rd(battery.QUIT_CURRENT_THRESH)[:2])[0])
    log.info('FET Options: 0x%02x', struct.unpack('<B', battery.df_rd(battery.FET_OPTIONS)[:1])[0])
    log.info('SOC_FLAG_CONFIG_A: 0x%04x', struct.unpack('<H', battery.df_rd(battery.SOC_FLAG_CONFIG_A)[:2])[0])
    log.info('SOC_FLAG_CONFIG_B: 0x%02x', struct.unpack('<B', battery.df_rd(battery.SOC_FLAG_CONFIG_B)[:1])[0])
    log.info('CHARGE_TERM_VOLTAGE: %d mV', struct.unpack('<h', battery.df_rd(battery.CHARGE_TERM_VOLTAGE)[:2])[0])
    log.info('SBS_GAUGING_CONF: 0x%02x', struct.unpack('<B', battery.df_rd(battery.SBS_GAUGING_CONF)[:1])[0])
    log.info('IT_GAUGING_CONF: 0x%04x', struct.unpack('<H', battery.df_rd(battery.IT_GAUGING_CONF)[:2])[0])

    term_voltage = struct.unpack('<h', battery.df_rd(battery.TERM_VOLTAGE)[:2])[0]
    log.info('3.6 Term Voltage: %d mv', term_voltage)
    update_status = struct.unpack('<B', battery.df_rd(battery.UPDATE_STATUS)[:1])[0]
    log.info('\tUpdate Status: 0x%02x', update_status)

    log.info('-')
    mfg_stat = battery.manufacturing_status()
    log.info('4.2.1 GAUGE_EN: %s  FET_EN: %s', mfg_stat['gauge'], mfg_stat['FET'])
    battery.device_reset()
    log.info('\tDevice reset')
    battery.unseal_device()
    log.info('\tDevice is unsealed.')
    log.info('\tRDIS: %s', mask_test(battery.it_status_reg(), battery.IT_STAT_RDIS))
    log.info('\tUpdate Status: 0x%02x', struct.unpack('<B', battery.df_rd(battery.UPDATE_STATUS)[:1])[0])

    gg_stat = battery.DataFlashGasGauge.read_pack(battery)
    log.info('-'*30)
    log.info('Gas Gauging Status:')
    for cmd in gg_stat._struct_info:
        log.info('\t%s: %d', cmd[0], getattr(gg_stat, cmd[0]))
    log.info('-'*30)

    handle_data_logs(battery)

if __name__ == "__main__":
    run_example()
