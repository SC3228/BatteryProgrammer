from ee.equipment.linux_smbdat import LinuxSmbDat
from ee.equipment.bq28z610 import BQ28Z610
import pickle
import time

BQ28Z610_DF_BASE_ADDRESS = 0x4000
BQ28Z610_DF_LENGTH = 0x2000

interface = LinuxSmbDat(1)
battery = BQ28Z610(interface=interface)  # Create a Battery Instance

print("Unsealing Device...")
battery.unseal_device()  # Unseal

print("Getting Pack Info...")
device_number, version, build_number, *__ = battery.get_firmware_info()  # Get Firmware Info
fw_version = battery.mfg_blk_rd(battery.FIRMWARE_VERSION)[3:5]
device_type = battery.mfg_blk_rd(battery.DEVICE_TYPE)
hw_version = battery.mfg_blk_rd(battery.HARDWARE_VERSION)

print("Reading DF Data...")
df = battery.read_flash_data(BQ28Z610_DF_BASE_ADDRESS,
                             BQ28Z610_DF_BASE_ADDRESS + BQ28Z610_DF_LENGTH - 1)

print("Sealing Device...")
battery.alt_mfg_cmd(battery.SEAL_DEVICE)
battery.device_reset()

pack = {
    'Data Flash': df,
    'FW Version': fw_version,
    'Device Type': device_type,
    'HW Version': hw_version
}

filename = f'BQ28Z610_Data_{time.strftime("%Y_%m_%d_%H_%M_%S")}.pkl'  # Filename can be changed here

print(f"Writing Data to file under name {filename}...")
with open(filename, 'wb') as f:
    pickle.dump(pack, f)

print("Done.")
