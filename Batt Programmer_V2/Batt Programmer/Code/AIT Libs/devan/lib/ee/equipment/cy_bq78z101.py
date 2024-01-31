import sys
sys.path.append('''/home/pi/zebrabattst/devan/lib''')
from .cy_smbdat import CyI2cSmbus, CySmbDat
from .smbdat import RolexBattery

class CyBQ78Z101(RolexBattery):
    def __init__(self, device_name=None, log=None, interface=None, bd_serial=None):
        if interface is None:
            device = CyI2cSmbus(serial=bd_serial)
            interface = CySmbDat(device=device, log=log)
        super(CyBQ78Z101, self).__init__(device_name, log, interface)

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
