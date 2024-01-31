import os

from .usb import ATMEL_CA_VID, ATMEL_CA_PID
from ...error import DeviceNotFoundError


__all__ = ['AtmelCryptoKitHIDLinux', 'enumerate_kit_devices_linux', 'connect_kit_device_linux']


USB_SYSFS_DEVICE_ROOT = '/sys/bus/usb/devices'
DEVICE_ROOT = '/dev'


class AtmelCryptoKitHIDLinux:
    def __init__(self, dev_node):
        self._fd = os.open(dev_node, os.O_RDWR)
        # This was fixing thigns... earlier. Now it seems to be breaking things. I hate computers.
        # self.send(b'\n')

    def send(self, tx_data):
        # The first byte is the HID report number - 0 if unused.
        os.write(self._fd, b'\x00' + tx_data)

    def recv(self):
        buf = b''

        while True:
            # The kernel seems to be sticking me with chunks of 64 NUL padded
            # anyway. This is basically just readline() though.
            data = os.read(self._fd, 64)
            idx = data.find(b'\n')

            if idx != -1:
                buf += data[:idx]

                return buf
            else:
                buf += data


def _parse_usb_uevent(path):
    """
    Given the path to a sysfs uevent file, return the key/value pairs as a dict.

    Lines in the uevent file are of the format::

        KEY=VALUE
    """
    with open(path, 'rb') as fh:
        return {a: b for a, b in [x.split('=') for x in fh.read().decode('utf8').strip().splitlines()]}


def _find_node_by_uevent(root, **match_uevent):
    """
    Return a list of subdirectories of ``root`` that have
    ``root/subdir/uevent`` files with the key/value pairs given in the
    ``match_uevent`` argument.
    """
    nodes = []

    for path in os.listdir(root):
        full_path = os.path.join(root, path)
        full_path_uevent = os.path.join(full_path, 'uevent')

        if os.path.exists(full_path_uevent):
            try:
                uevent = _parse_usb_uevent(full_path_uevent)
            except IOError:
                continue

            matches = True
            for key in match_uevent:
                if key not in uevent or uevent[key] != match_uevent[key]:
                    matches = False
                    break

            if matches:
                nodes.append((full_path, uevent))

    return nodes


def _enumerate_usbhid_nodes(root):
    """
    Gather a list of sysfs directories corresponding to USB interfaces to which
    the usbhid kernel module is attached.

    e.g.::
        [/sys/bus/usb/devices/1-2.2:1.0]
    """
    return _find_node_by_uevent(root, DRIVER='usbhid')

def _find_hidraw_devs(hidgeneric_root, dev_root):
    """
    Given the path to an ``hid-generic`` directory, find the ``hidraw``
    devices.
    """
    hidraw_root = os.path.join(hidgeneric_root, 'hidraw')

    if not os.path.isdir(hidraw_root):
        return []


    devs = []
    for name in os.listdir(hidraw_root):
        uevent_path = os.path.join(hidraw_root, name, 'uevent')

        if os.path.exists(uevent_path):
            uevent = _parse_usb_uevent(uevent_path)

            devs.append(os.path.join(dev_root, uevent['DEVNAME']))

    return devs

def enumerate_kit_devices_linux():
    """
    Enumerate kit devices. Each item in the returned list is an *opaque* object
    that can be connected to with ``connect_kit_device_linux``.
    """
    kit_devs = []

    for usbhid_node, usbhid_uevent in _enumerate_usbhid_nodes(USB_SYSFS_DEVICE_ROOT):

        # Now check the uevent file for the vid/pid
        vid_str, pid_str, bcd_str = usbhid_uevent['PRODUCT'].split('/')
        if int(vid_str, 16) != ATMEL_CA_VID or int(pid_str, 16) != ATMEL_CA_PID:
            continue

        # Now we want hid-generic sub directories
        for hidgeneric_node, hidgeneric_uevent in _find_node_by_uevent(usbhid_node, DRIVER='hid-generic'):
            kit_devs.extend(_find_hidraw_devs(hidgeneric_node, DEVICE_ROOT))

    return kit_devs


def connect_kit_device_linux(dev):
    """
    Connect to a kit device given a device returned by
    ``enumerate_kit_devices_linux``.
    """
    return AtmelCryptoKitHIDLinux(dev)
