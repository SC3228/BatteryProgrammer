from ctypes import * # pylint: disable=wildcard-import, unused-wildcard-import
from ctypes.wintypes import DWORD, WORD, HANDLE, BYTE

__all__ = [
    'CreateFile', 'SetupDiGetClassDevs', 'SetupDiEnumDeviceInterfaces',
    'SetupDiGetDeviceInterfaceDetail', 'GetLastError', 'SetupDiDestroyDeviceInfoList',
    'CloseHandle', 'WriteFile', 'ReadFile', 'DWORD', 'HANDLE', 'FILE_SHARE_READ',
    'FILE_SHARE_WRITE', 'GENERIC_READ', 'GENERIC_WRITE', 'DIGCF_DEVICEINTERFACE',
    'DIGCF_PRESENT', 'INVALID_HANDLE_VALUE', 'OPEN_EXISTING', 'HID_GUID',
    'SP_DEVICE_INTERFACE_DETAIL_DATA', 'SP_DEVICE_INTERFACE_DATA', 'NULL', 'WinError'
]


CreateFile = windll.kernel32.CreateFileA
CreateFile.restype = HANDLE

# XXX: 64-bit fix - for some damn reason this is set to c_long, which
# fine on 32-bit windows. But on 64-bit windows, ints and longs are...
# wait for it... both 32-bits, oh but by the way... pointers are 64
# bits so you can't stuff those in longs. Truncation is bad, mmmkay?
SetupDiGetClassDevs = windll.setupapi.SetupDiGetClassDevsA
SetupDiGetClassDevs.restype = HANDLE

SetupDiEnumDeviceInterfaces = windll.setupapi.SetupDiEnumDeviceInterfaces
SetupDiGetDeviceInterfaceDetail = windll.setupapi.SetupDiGetDeviceInterfaceDetailA
GetLastError = windll.kernel32.GetLastError
SetupDiDestroyDeviceInfoList = windll.setupapi.SetupDiDestroyDeviceInfoList
CloseHandle = windll.kernel32.CloseHandle
WriteFile = windll.kernel32.WriteFile
ReadFile = windll.kernel32.ReadFile


# XXX: This is "wrong" - it *should* be an integer type: 64-bit on 64-bit
# windows, 32-bit on 32-bit...
ULONG_PTR = c_void_p
NULL = c_void_p(0)

# /usr/i686-w64-mingw32/include/setupapi.h
DIGCF_DEVICEINTERFACE = 0x00000010
DIGCF_PRESENT = 0x00000002

# XXX: WTF? Shouldn't this be c_void_p(something)?
INVALID_HANDLE_VALUE = -1

# /usr/i686-w64-mingw32/include/winnt.h
FILE_SHARE_READ = 0x00000001
FILE_SHARE_WRITE = 0x00000002
GENERIC_READ = 0x80000000
GENERIC_WRITE = 0x40000000

# https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858%28v=vs.85%29.aspx
OPEN_EXISTING = 0x03


class GUID(Structure):
    """
    https://msdn.microsoft.com/en-us/library/windows/desktop/aa373931%28v=vs.85%29.aspx
    """
    _fields_ = [
        ('data1', DWORD),
        ('data2', WORD),
        ('data3', WORD),
        ('data4', BYTE * 8)
    ]

    def __init__(self, data1, data2, data3, *args):
        super(GUID, self).__init__()
        assert len(args) == 8

        self.data1 = data1
        self.data2 = data2
        self.data3 = data3

        for idx in range(len(args)):
            self.data4[idx] = args[idx]


# /usr/i686-w64-mingw32/include/setupapi.h
class SP_DEVICE_INTERFACE_DATA(Structure):
    _fields_ = [
        ('cbSize', DWORD),
        ('InterfaceClassGuid', GUID),
        ('Flags', DWORD),
        ('Reserved', ULONG_PTR)
    ]

    def __init__(self):
        super(SP_DEVICE_INTERFACE_DATA, self).__init__()
        memset(byref(self), 0, sizeof(self))
        self.cbSize = sizeof(self)


def SP_DEVICE_INTERFACE_DETAIL_DATA(size):

    class _SP_DEVICE_INTERFACE_DETAIL_DATA(Structure):
        _fields_ = [
            ('cbSize', DWORD),
            ('DevicePath', c_char * max(size - sizeof(DWORD), 1))
        ]

        def __init__(self):
            super(_SP_DEVICE_INTERFACE_DETAIL_DATA, self).__init__()
            # First problem: ctypes does not support variable length array
            # members (which normal people define with char foo[] in c99,
            # equivalent to foo[0]).
            #
            # Solution: This function that wraps the class and creates a
            # Structure with the correct member size.
            #
            # Second problem: Windows actually uses foo[1] instead of foo[0].
            # Solution: Add 1.
            #
            # Biggest problem: cbSize MUST be:
            #     sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA)
            # We cannot get this value easily because sizeof(self) would
            # include the full size of DevicePath as well. So then you might
            # think that you could just use
            # sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA(1), but you would be
            # wrong. That comes out to 8 (3 bytes padding for a 2*DWORD stride
            # length). Windows actually packs structures and doesn't tell you
            # about it, so the correct value is 5. Ah, but it only secretly
            # packs structures in 32-bit environments, so on 64-bit, the value is 8...

            # XXX: due to above crap, detecting 64 vs 32 bit via sizeof(c_void_p)
            self.cbSize = 8 if (sizeof(c_void_p) == 8) else 5

    return _SP_DEVICE_INTERFACE_DETAIL_DATA()


HID_GUID = GUID(0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30)
