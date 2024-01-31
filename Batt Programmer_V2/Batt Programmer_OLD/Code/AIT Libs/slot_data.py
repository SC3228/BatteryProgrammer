#!/usr/local/bin/python
# coding: latin-1
#added lou
import sys
#sys.path.append('''/home/pi/AIT/lib''')
sys.path.append('''AIT/lib''')
#sys.path.append('''/home/pi/aittest/AIT/lib''')

import zstruct
from hexdump import hexdump
import struct

from ee.equipment import atcalib

Type = zstruct.Type

try:
    xrange
except NameError:
    xrange = range

try:
    raw_input
except NameError:
    raw_input=input

'''
Invalid Test String:
:1000000002FF5A45425241A9323031345031303822
:10001000333232373700000030413237323136491F
:100020004A543030303153500000090201200000A2
:10003000000000010008EB574130353030333638CE
:100040003235574F723CA006010101FFFFFFFFFF51
:10005000FFFFFFFFFFFFFFFFFFFFFFFFFFFFA6AB5D
:100060000002E8085E06140F0D59B60A6C0CEE0982
:100070000C0D0209E0156C0C5F0648036C0C5E0663
:1000800048036C0C5E064803800964FFFFFFFFFF16
:100090004E434136353338363453FFFFFFFFFFFF01
:00000001FF
'''

VENDOR_CODE_PANASONIC = 0X07
VENDOR_CODE_INVENTUS = 0X08
VENDOR_CODE_TWS = 0X09

COO_NONE = 0X00
COO_JAPAN = 0X01
COO_CHINA = 0X02
COO_VIETNAM = 0X03
COO_USA = 0X04
COO_MEXICO = 0X05
COO_THAILAND = 0X06

TYPE_BATTERY = 0x2001

REGION_WW = '\x00'
REGION_EMERGING = '\x01'

class BatteryEEPROM(object):
    def __init__(self, at508a_dev=None, i2c_dev=None):
        if at508a_dev is not None:
            self.dev = at508a_dev
#        elif i2c_dev is not None:
#            self.dev = atcalib.atca_i2c_cypress(i2c_dev_node=i2c_dev)
#        else:
#            self.dev = atcalib.atca_i2c_cypress()
            
    def get_slot_8(self):
        slot_data = bytearray()
        for block in range(13):
            slot_data += self.dev.read_zone(2, 8, block, 0, 32)
            
        return Slot8BatteryData(slot_data[:len(Slot8BatteryData)])
        
    def get_slot_7(self):
        slot_data = bytearray()
        slot_data += self.dev.read_zone(2, 7, 0, 0, 32)
        slot_data += self.dev.read_zone(2, 7, 1, 0, 4)
        
        return Slot7Data(slot_data[:len(Slot7Data)])
    

class Slot8HeaderData(zstruct.Struct):
    """
    Zebra Consumable device header data.
    """
    _format = zstruct.Format.LittleEndian

    data_revision = Type.UnsignedByte
    pad0 = Type.UnsignedByte
    copyright_string = Type.String[10]
    part_number = Type.String[12]
    consumable_revision = Type.String[2]
    serial_number = Type.String[16]
    vendor_code = Type.UnsignedByte
    country_of_origin = Type.UnsignedByte
    type_number = Type.UnsignedShort
    oem_code = Type.String[6]
    date_of_mfg = Type.UnsignedLong
    mfg_lot = Type.UnsignedByte[6]
    consumable_lot = Type.UnsignedByte[6]
    purchase_order = Type.UnsignedByte[4]
    programmer_id = Type.UnsignedByte
    station_id = Type.UnsignedByte
    operator_id = Type.UnsignedByte
    reservedh0 = Type.UnsignedByte[19]
    crch_16 = Type.UnsignedShort
    reservedh1 = Type.UnsignedByte[32]


assert len(Slot8HeaderData) == 128


class Slot8BatteryData(Slot8HeaderData):
    """
    Battery data stored in slot8 of P10832277 for power precision.
    """
    _format = zstruct.Format.LittleEndian

    data_rev = Type.UnsignedByte
    pp_ver = Type.UnsignedByte
    design_capacity = Type.UnsignedShort
    max_charge_rate = Type.UnsignedShort
    soc_warn_faire = Type.UnsignedByte
    soc_warn_unfaire = Type.UnsignedByte
    soc_warn_low = Type.UnsignedByte
    recharge_thres = Type.UnsignedByte
    chg_temp_low = Type.UnsignedShort
    chg_temp_high = Type.UnsignedShort
    dch_temp_low = Type.UnsignedShort
    dch_temp_high = Type.UnsignedShort
    chg_slow_to = Type.UnsignedByte
    chg_fast_to = Type.UnsignedByte
    chg_pre_thres = Type.UnsignedShort
    chg_scl_temp_1 = Type.UnsignedShort
    chg_scl_rate_1 = Type.UnsignedShort
    chg_scl_volt_1 = Type.UnsignedShort
    chg_scl_temp_2 = Type.UnsignedShort
    chg_scl_rate_2 = Type.UnsignedShort
    chg_scl_volt_2 = Type.UnsignedShort
    chg_scl_temp_3 = Type.UnsignedShort
    chg_scl_rate_3 = Type.UnsignedShort
    chg_scl_volt_3 = Type.UnsignedShort
    no_battery_temp = Type.UnsignedShort
    chg_term_rate = Type.UnsignedByte
    pad1 = Type.UnsignedByte[5]
    cell_model = Type.String[10]
    reserved0 = Type.UnsignedByte[6]
    reserved1 = Type.UnsignedByte[32]
    reserved2 = Type.UnsignedByte[32]
    reserved3 = Type.UnsignedByte[32]
    reserved4 = Type.UnsignedByte[32]
    reserved5 = Type.UnsignedByte[32]
    reserved6 = Type.UnsignedByte[32]
    reserved7 = Type.UnsignedByte[14]
    crc_16 = Type.UnsignedShort

assert len(Slot8BatteryData) == 400

class Slot7Data(zstruct.Struct):
    _format = zstruct.Format.LittleEndian

    data_revision = Type.UnsignedByte
    dtp = Type.UnsignedByte
    reserved = Type.UnsignedByte[32]
    crc_16 = Type.UnsignedShort
    
assert len(Slot7Data) == 36

import binascii

TranslateList = ['date_of_mfg',
                 'crch_16']

IgnoreList = ['serial_number',
              'date_of_mfg',
              'mfg_lot',
              'consumable_lot',
              'purchase_order',
              'programmer_id',
              'station_id',
              'operator_id',
              'crch_16']

D10011324_HEADER_WW_TWS = Slot8HeaderData(data_revision=3,
                                          pad0=0xFF,
                                          copyright_string='ZEBRA©2014'.encode('utf-8'),
                                          part_number='P10832277\x00\x00\x00'.encode('utf-8'),
                                          consumable_revision='0A'.encode('utf-8'),
                                          serial_number=b'\xFF' * 16,
                                          vendor_code=VENDOR_CODE_TWS,
                                          country_of_origin=COO_CHINA,
                                          type_number=TYPE_BATTERY,
                                          oem_code=(b'\x00' * 5) + b'\x00',
                                          date_of_mfg=0xFFFF,
                                          mfg_lot=[0xFF] * 6,
                                          consumable_lot=[0xFF] * 6,
                                          purchase_order=[0xFF] * 4,
                                          programmer_id=0xFF,
                                          station_id=0xFF,
                                          operator_id=0xFF,
                                          reservedh0=[0xFF] * 19,
                                          crch_16=0xFFFF,
                                          reservedh1=[0xFF] * 32)

D10012057_DATA_WW_TWS = Slot8BatteryData(bytes(D10011324_HEADER_WW_TWS) +
                                         bytes(b'\xff' * (len(Slot8BatteryData) - len(Slot8HeaderData))))
D10012057_DATA_WW_TWS.data_rev = 0
D10012057_DATA_WW_TWS.pp_ver = 2
D10012057_DATA_WW_TWS.design_capacity = 2280
D10012057_DATA_WW_TWS.max_charge_rate = 1630
D10012057_DATA_WW_TWS.soc_warn_faire = 20
D10012057_DATA_WW_TWS.soc_warn_unfaire = 15
D10012057_DATA_WW_TWS.soc_warn_low = 13
D10012057_DATA_WW_TWS.recharge_thres = 89
D10012057_DATA_WW_TWS.chg_temp_low = 2742
D10012057_DATA_WW_TWS.chg_temp_high = 3180
D10012057_DATA_WW_TWS.dch_temp_low = 2542
D10012057_DATA_WW_TWS.dch_temp_high = 3340
D10012057_DATA_WW_TWS.chg_slow_to = 2
D10012057_DATA_WW_TWS.chg_fast_to = 9
D10012057_DATA_WW_TWS.chg_pre_thres = 5600
D10012057_DATA_WW_TWS.chg_scl_temp_1 = 3180
D10012057_DATA_WW_TWS.chg_scl_rate_1 = 1630
D10012057_DATA_WW_TWS.chg_scl_volt_1 = 840
D10012057_DATA_WW_TWS.chg_scl_temp_2 = 3180
D10012057_DATA_WW_TWS.chg_scl_rate_2 = 1630
D10012057_DATA_WW_TWS.chg_scl_volt_2 = 840
D10012057_DATA_WW_TWS.chg_scl_temp_3 = 3180
D10012057_DATA_WW_TWS.chg_scl_rate_3 = 1630
D10012057_DATA_WW_TWS.chg_scl_volt_3 = 840
D10012057_DATA_WW_TWS.no_battery_temp = 2432
D10012057_DATA_WW_TWS.chg_term_rate = 100
D10012057_DATA_WW_TWS.pad1 = [0xFF] * 5
D10012057_DATA_WW_TWS.cell_model = 'NCA653864S'
D10012057_DATA_WW_TWS.reserved0 = [0xFF] * 6
D10012057_DATA_WW_TWS.reserved1 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved2 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved3 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved4 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved5 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved6 = [0xFF] * 32
D10012057_DATA_WW_TWS.reserved7 = [0xFF] * 14
D10012057_DATA_WW_TWS.crc_16 = 0xFFFF


CRC16_CCITT_SIZE = 2
CRC16_CCITT_POLY = 0x1021
CRC16_CCITT_INIT = 0xFFFF


def cksum_crc16_ccitt(data, starting_crc = CRC16_CCITT_INIT):
    """ Return the CRC16-CCITT of the input data. """
    crc = starting_crc

    for byte in bytearray(data):
        msg = (byte << 8) & 0xFFFF

        for _unused in xrange(8):
            if (msg ^ crc) >> 15:
                crc = ((crc << 1) ^ CRC16_CCITT_POLY) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

            msg = (msg << 1) & 0xFFFF

    return crc

# Test the full and running checksum.
assert 0xeb11 == cksum_crc16_ccitt("hammysammy".encode('utf-8'))
assert 0xeb11 == cksum_crc16_ccitt("sammy".encode('utf-8'), cksum_crc16_ccitt("hammy".encode('utf-8')))


def dump_fields(test, expected_struct):
    width = 16
    offset = 0

    test_bytes = bytes(test)
    expd_bytes = bytes(expected_struct)

    print('      ', end='')
    for i in range(width):
        print('{:0>2X}     '.format(i), end='')

    for name, my_format in expected_struct._struct_info:
        length = my_format.size
        val = getattr(test, name)
        exp = getattr(expected_struct, name)

        for byte in range(length):
            if offset % width == 0:
                print('\n{:0>4X}: '.format(offset), end='')
            if name in IgnoreList:
                print('{:>2}     '.format('XX'), end='')
            elif test_bytes[offset] != expd_bytes[offset]:
                print('{:0>2X}({:0>2X}) '.format(test_bytes[offset], expd_bytes[offset]), end='')
            else:
                try:
                    print('{:0>2X}     '.format(test_bytes[offset]), end='')
                except TypeError:
                    print('++++' + repr(str(test_bytes[offset])) + '++++')
                    exit(0)

            offset += 1
    print('\n')

def report_fields(test, expected_struct):
    # Validate fields
    for v in Slot8HeaderData._struct_info:
        val = getattr(test, v[0])
        # # print(v[0], type(val), val)
        # if type(val) is int:
        #     print(v[0], hex(val))
        # elif type(val) is list:
        #     print(v[0], [hex(x) for x in val])
        # else:
        #     print(v[0], val)

        if v[0] not in IgnoreList:
            expected_val = getattr(expected_struct, v[0])
            if val != expected_val:
                print('%% MISMATCH({}): expected({}) got({}) %%'.format(v[0], expected_val, val))

import re

def validate(header_requirements, data_requirements):
    print(('Populating data...'))
    input_data = bytearray()
    try:
        while len(input_data)/2 < len(Slot8BatteryData):
            count = 0
            line = raw_input()
            if line[0] == ':':
                line = line[:-2]
            line = re.sub('^:.{8}', '', line)
            print(type(input_data))
            print(type(line))
            input_data.extend(line.encode('utf-8'))
            print(('{} bytes loaded'.format(len(input_data)/2)))
    except EOFError:
        pass
    input_data = bytes(input_data)

    input_data = binascii.unhexlify(input_data.replace(b':', b''))

    print(('Loaded data:'))
    hexdump(input_data)

    '''
    Validate Consumable Header
    '''
    print(('*' * 20))
    print(('Header data only:'))
    try:
        data = Slot8HeaderData(input_data[:len(Slot8HeaderData)])
    except struct.error:
        print('Failed to load header data (check formatting)')
    else:
        report_fields(data, header_requirements)
        dump_fields(data, header_requirements)

        # Validate crc
        expected = cksum_crc16_ccitt(input_data[:0x5E])
        val = getattr(data, 'crch_16')
        if expected != val:
            print('%% CRC MISMATCH: expected({:X}) got({:X}) %%'.format(expected, val))

    '''
    Validate Battery Data
    '''
    print(('*' * 20))
    print(('Battery data:'))
    try:
        data = Slot8BatteryData(input_data[:len(Slot8BatteryData)])
    except struct.error:
        print('Failed to load data to struct (check formatting)')
    else:
        dump_fields(data, data_requirements)

        # Validate crc
        expected = cksum_crc16_ccitt(input_data[0x80:0x18E])
        val = getattr(data, 'crc_16')
        if expected != val:
            print('%% CRC MISMATCH: expected({:X}) got({:X}) %%'.format(expected, val))

if __name__ == "__main__":
    a = BatteryEEPROM(at508a_dev = atcalib.atca_i2c_linux("/dev/i2c-1"))
#    a = BatteryEEPROM(at508a_dev = atcalib.atca_i2c_linux("/dev/i2c-3"))
    slot_8 = a.get_slot_8()
#    slot_7 = a.get_slot_7()
    print("Serial Number -> ",slot_8.serial_number)
    print("Date of mfg -> ",slot_8.date_of_mfg )
    print("Part Number -> ",slot_8.part_number)
    print("Max_charge_rate -> ",slot_8.max_charge_rate)
    print("Chr_term_rate -> ",slot_8.chg_term_rate)
