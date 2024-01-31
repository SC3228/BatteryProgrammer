import platform

from ctypes import byref, create_string_buffer, memset, memmove, sizeof, addressof

from .usb import ATMEL_CA_VID, ATMEL_CA_PID

if platform.system() == 'Windows':
    from .hid_windows_platform import * # pylint: disable=wildcard-import


__all__ = ['enumerate_kit_devices_windows', 'connect_kit_device_windows', 'AtmelCryptoKitHIDWindows']


class AtmelCryptoKitHIDWindows:
    """
    :param device_path: a USB device path.
    """

    def __init__(self, device_path):
        self._rh, self._wh = self._connect_hid_dev(device_path)

    @staticmethod
    def _connect_hid_dev(device_path):
        """
        Open read and write handles to a USB device given the device path.
        """
        rh = CreateFile(device_path, DWORD(GENERIC_READ), DWORD(FILE_SHARE_READ | FILE_SHARE_WRITE),
                        NULL, DWORD(OPEN_EXISTING), DWORD(0), NULL)

        if rh == INVALID_HANDLE_VALUE:
            raise WinError()

        wh = CreateFile(device_path, DWORD(GENERIC_WRITE), DWORD(FILE_SHARE_READ | FILE_SHARE_WRITE),
                        NULL, DWORD(OPEN_EXISTING), DWORD(0), NULL)

        if wh == INVALID_HANDLE_VALUE:
            CloseHandle(rh)
            raise WinError()

        return rh, wh

    def send(self, tx_data):
        tx_idx = 0

        # XXX: 65 hard-coded
        buf = create_string_buffer(65)
        while tx_idx < len(tx_data):
            memset(buf, 0, sizeof(buf))

            bytes_left = len(tx_data) - tx_idx
            bytes_to_send = min(64, bytes_left)

            # NOTE: buf + 1: HID REPORT NUMBER
            memmove(addressof(buf) + 1, tx_data[tx_idx:tx_idx + bytes_to_send], bytes_to_send)

            # TODO check my returns
            bytes_sent = DWORD(0)
            result = WriteFile(HANDLE(self._wh), buf, DWORD(sizeof(buf)), byref(bytes_sent), NULL)

            if result == 0:
                raise WinError()

            #if result == 0?
            #epic fail!

            tx_idx += bytes_to_send

    def recv(self):
        rx_data = b''
        buf = create_string_buffer(65)
        bytes_read = DWORD(0)

        continue_read = True
        while continue_read:
            result = ReadFile(self._rh, buf, sizeof(buf), byref(bytes_read), NULL)
            rx_bytes = buf[:bytes_read.value]

            # if result == 0...
            idx = rx_bytes.find(b'\n')

            if idx != -1:
                continue_read = False
                end_idx = idx
            else:
                end_idx = len(rx_bytes)

            # NOTE: HID report number is byte 0
            rx_data += rx_bytes[1:end_idx]

        return rx_data

    def __del__(self):
        try:
            CloseHandle(self._wh)
        except: # pylint: disable=bare-except
            pass

        try:
            CloseHandle(self._rh)
        except: # pylint: disable=bare-except
            pass


def enumerate_kit_devices_windows():
    """
    Return a a list of kit devices. Each item in the list should be considered
    an *opaque* object that can only be accessed with functions such as
    ``connect_kit_device_windows``.
    """
    dev_info = SetupDiGetClassDevs(byref(HID_GUID), NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)

    if dev_info == INVALID_HANDLE_VALUE:
        return []

    kit_devs = []
    kit_hid_filter = ('vid_%04x&pid_%04x' % (ATMEL_CA_VID, ATMEL_CA_PID)).encode('ascii')

    try:
        for dev_detail_data in _iter_hid_devs(dev_info):

            if kit_hid_filter in dev_detail_data.DevicePath:
                kit_devs.append(bytes(dev_detail_data.DevicePath))
    finally:
        SetupDiDestroyDeviceInfoList(HANDLE(dev_info))

    return kit_devs

def connect_kit_device_windows(dev):
    """
    Connect to a kit device as returned by ``enumerate_kit_devices_windows``.
    """
    return AtmelCryptoKitHIDWindows(dev)

def _iter_hid_devs(dev_info):
    """
    Given a device info handle (``dev_info``), iterate over HID devices
    attached to the system, yielding an ``SP_DEVICE_INTEFFACE_DETAIL_DATA``
    structure on each iteration.
    """
    # allocate a currently empty structure to put device data in
    dev_data = SP_DEVICE_INTERFACE_DATA()
    device_index = 0

    # Find all Atmel Kit USB devcies?
    while True:
        result = SetupDiEnumDeviceInterfaces(HANDLE(dev_info), NULL, byref(HID_GUID),
                                             DWORD(device_index), byref(dev_data))

        if result == 0:
            # 259: when there are no more devices
            if GetLastError() == 259:
                break
            else:
                raise WinError()

        # The first call to SetupDiGetDeviceInterfaceDetail merely
        # populates required_size, telling us how much memory to allocate
        required_size = DWORD(0)
        SetupDiGetDeviceInterfaceDetail(HANDLE(dev_info), byref(dev_data), NULL, required_size,
                                        byref(required_size), NULL)

        dev_detail_data = SP_DEVICE_INTERFACE_DETAIL_DATA(required_size.value)

        result = SetupDiGetDeviceInterfaceDetail(HANDLE(dev_info), byref(dev_data), byref(dev_detail_data),
                                                 required_size, byref(required_size), NULL)

        # TODO: BAH Y U NO CHECK RETURNS?
        # (the Atmel code didn't either. self.#REKT)

        yield dev_detail_data

        device_index += 1
