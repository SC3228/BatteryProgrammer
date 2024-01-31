# Rev D

from ee.utils.slot_data import *

D10012268_HEADER_RG_INVENTUS = Slot8HeaderData(data_revision=3,
                                               pad0=0xFF,
                                               copyright_string=b'ZEBRA\xa92014',
                                               part_number=b'P1089760-002',
                                               consumable_revision=b'0A',
                                               serial_number=b'\xFF' * 16,
                                               vendor_code=VENDOR_CODE_INVENTUS,
                                               country_of_origin=COO_CHINA,
                                               type_number=TYPE_BATTERY,
                                               oem_code=(('\x00' * 5) + REGION_WW).encode('utf-8'),
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
D10012268_DATA_WW_INVENTUS = Slot8BatteryData(bytes(D10012268_HEADER_RG_INVENTUS) +
                                              bytes(b'\xff' * (len(Slot8BatteryData) - len(Slot8HeaderData))))


D10012268_DATA_WW_INVENTUS.data_rev = 2
D10012268_DATA_WW_INVENTUS.pp_ver = 2
D10012268_DATA_WW_INVENTUS.design_capacity = 0x1a90
D10012268_DATA_WW_INVENTUS.max_charge_rate = 0x0BB8
D10012268_DATA_WW_INVENTUS.soc_warn_faire = 8
D10012268_DATA_WW_INVENTUS.soc_warn_unfaire = 4
D10012268_DATA_WW_INVENTUS.soc_warn_low = 2
D10012268_DATA_WW_INVENTUS.recharge_thres = 89
D10012268_DATA_WW_INVENTUS.chg_temp_low = 2732
D10012268_DATA_WW_INVENTUS.chg_temp_high = 3181     # 45 degrees Celcius
D10012268_DATA_WW_INVENTUS.dch_temp_low = 2531
D10012268_DATA_WW_INVENTUS.dch_temp_high = 3331
D10012268_DATA_WW_INVENTUS.chg_slow_to = 2
D10012268_DATA_WW_INVENTUS.chg_fast_to = 9
D10012268_DATA_WW_INVENTUS.chg_pre_thres = 5500     # 45 degrees Celcius
D10012268_DATA_WW_INVENTUS.chg_scl_temp_1 = 2831
D10012268_DATA_WW_INVENTUS.chg_scl_rate_1 = 0x0d48
D10012268_DATA_WW_INVENTUS.chg_scl_volt_1 = 0x0334
D10012268_DATA_WW_INVENTUS.chg_scl_temp_2 = 3181    # 45 degrees Celcius
D10012268_DATA_WW_INVENTUS.chg_scl_rate_2 = 0x1A90
D10012268_DATA_WW_INVENTUS.chg_scl_volt_2 = 840
D10012268_DATA_WW_INVENTUS.chg_scl_temp_3 = 3181    # 45 degrees Celcius
D10012268_DATA_WW_INVENTUS.chg_scl_rate_3 = 0x1A90
D10012268_DATA_WW_INVENTUS.chg_scl_volt_3 = 840
D10012268_DATA_WW_INVENTUS.no_battery_temp = 2432
D10012268_DATA_WW_INVENTUS.chg_term_rate = 250
D10012268_DATA_WW_INVENTUS.pad1 = [0xFF] * 5
D10012268_DATA_WW_INVENTUS.cell_model = b'INR1865MJ1'
D10012268_DATA_WW_INVENTUS.reserved0 = [0xFF] * 6
D10012268_DATA_WW_INVENTUS.reserved1 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved2 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved3 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved4 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved5 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved6 = [0xFF] * 32
D10012268_DATA_WW_INVENTUS.reserved7 = [0xFF] * 14
D10012268_DATA_WW_INVENTUS.crc_16 = 0xFFFF

validate(D10012268_HEADER_RG_INVENTUS, D10012268_DATA_WW_INVENTUS)