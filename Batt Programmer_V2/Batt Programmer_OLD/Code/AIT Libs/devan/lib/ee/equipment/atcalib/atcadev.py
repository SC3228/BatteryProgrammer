"""
High level, PHY-independent abstraction for communicating with an Atmel ECCx08
crypto authentication device.
"""

import time
import struct

from .error import *

ECCX08_CRC_POLY = 0x8005

ECCX08_CMD_READ = 0x02
ECCX08_CMD_RANDOM = 0x1B
ECCX08_CMD_MAC = 0x08

ECCX08_ZONE_CONFIG = 0x00
ECCX08_ZONE_OTP = 0x01
ECCX08_ZONE_DATA = 0x02

ECCX08_STATUS_SUCCESS = 0x00
ECCX08_STATUS_MISCOMPARE = 0x01
ECCX08_STATUS_PARSE_ERROR = 0x03
ECCX08_STATUS_ECC_FAULT = 0x05
ECCX08_STATUS_EXECUTION_ERROR = 0x0F
ECCX08_STATUS_WAKE = 0x011
ECCX08_STATUS_WATCHDOG_EXPIRE = 0xEE
ECCX08_STATUS_COMM_ERROR = 0xFF

#lou
ATCA_I2C_DEFAULT_ADDRESS = 0xC2
#ATCA_I2C_DEFAULT_ADDRESS = 0x61

def _atca_calculate_crc(data, poly=ECCX08_CRC_POLY):
    """
    Calculate a command or response packet CRC. This is used for all
    commands sent to the device (mode 0x03) and all responses from the device.

    :param data: a bytes-like object.
    :param poly: The polynomial to use in the CRC calculations.
    :return: CRC16(data, poly). Only the low 16-bits are valid - the return
        MUST be packed before sending on the wire.
    """
    crc_register = 0

    for byte in data:

        for shift in range(0, 8):

            shift_register = 2 ** shift
            data_bit = 1 if (byte & shift_register) else 0
            crc_bit = (crc_register >> 15)
            crc_register <<= 1

            if data_bit != crc_bit:
                crc_register ^= poly

            crc_register &= 0xFFFF

    return crc_register


class AtmelCryptoDevice:
    """
    A high-level interface to an ECCX508 device.

    :param hal: The physical connection to the device used for all commands.
    """

    def __init__(self, hal):
        self._hal = hal

    def send_cmd(self, opcode, param1, param2, rx_size, delay, data=None):
        """
        See section 9.1 of the ECC508A datasheet.
        """
        # The general format for a command is:
        # uint8_t total length (including this byte and the CRC bytes!)
        # uint8_t param1 - this varies per command.

        # uint16_t param2 - this varies per command.
        # uint8_t[] data  - variable amount of data
        # uint16_t chksum - 16-bit CRC

        # TODO: it's so ghetto to make it wake up every command.
        # Immeasureable amounts of ghetto... but it works!
        try:
            self._hal.wakeup()
            #print("opcode,param1,param2",hex(opcode),hex(param1),hex(param2) )
            op_data = struct.pack('<BBH', opcode, param1, param2)
            #print("op_data",op_data)
            # +3 accounts for the length and checksum fields
            pkt_size = 3 + len(op_data)

            if data is not None:
                pkt_size += len(data)
                op_data += data

            cmd_pkt = struct.pack('<B', pkt_size) + op_data

            chksum = _atca_calculate_crc(cmd_pkt)
            cmd_pkt += struct.pack('<H', chksum)

            self._hal.send(cmd_pkt)

            # TODO: I don't really _like_ this
            time.sleep(delay/1000.0)

            raw_resp = self._hal.recv(rx_size)

        except OSError:
            raise
        finally:
            self._hal.sleep()
        
        time.sleep(0.1)
        
        # TODO - real exception
        assert len(raw_resp) == rx_size
        resp_data = self._parse_resp(raw_resp)

        # TODO: should we handle wake differently?
        exception_map = {
            ECCX08_STATUS_MISCOMPARE: MiscompareError,
            ECCX08_STATUS_PARSE_ERROR: CommandParseError,
            ECCX08_STATUS_ECC_FAULT: ECCFault,
            ECCX08_STATUS_WAKE: WakeupError,
            ECCX08_STATUS_WATCHDOG_EXPIRE: WatchDogExpire,
            ECCX08_STATUS_EXECUTION_ERROR: CommandExecutionError,
            ECCX08_STATUS_COMM_ERROR: CommError,
        }

        # If the length of the response is 1, it is a status or an error. No
        # other replies can ever be only one byte.
        if len(resp_data) == 1:
            status_code = resp_data[0]

            if status_code in exception_map:
                raise exception_map[status_code](code = status_code)

        return resp_data

    def _parse_resp(self, raw_resp):
        resp_len = raw_resp[0]

        # It *is* possible that the actual response is longer than the number
        # of bytes received in which case the trailing data is all garbage
        # (0xFF). We don't want garbage.
        resp_bytes = raw_resp[:resp_len]
        #lou
        #print("resp_bytes",resp_bytes)
        # Last two bytes are the expected checksum (little endian)
        expected_chksum = struct.unpack('<H', resp_bytes[-2:])[0]
        actual_chksum = _atca_calculate_crc(resp_bytes[:-2])

        if expected_chksum != actual_chksum:
            raise CRCValidationError("CRC mismatch on reply from device.")

        # The good response is everything between the count byte and checksum, not inclusive.
        resp_data = resp_bytes[1:-2]

        return resp_data

    @staticmethod
    def get_addr(zone, slot, block, offset):
        ATCA_ZONE_CONFIG = 0x00
        ATCA_ZONE_OTP = 0x01
        ATCA_ZONE_DATA = 0x02

        memzone = zone & 0x03;

        assert (memzone == ATCA_ZONE_CONFIG) or (memzone == ATCA_ZONE_DATA) or (memzone == ATCA_ZONE_OTP)

        # Initialize the addr to 00
        addr = 0;
        # Mask the offset
        offset = offset & 0x07;
        if (memzone == ATCA_ZONE_CONFIG) or (memzone == ATCA_ZONE_OTP):
            addr = block << 3;
            addr |= offset;
        else:  # ATCA_ZONE_DATA
            addr = slot << 3;
            addr  |= offset;
            addr |= block << 8;

        return addr;
        
    def read_config(self, addr, size):
        """
        A high-level read method for the configuration zone.

        This is intended to simplify reading by allowing reads of arbitrary
        lengths and addresses that are not word-aligned. This method performs
        the fewest reads possible to get the required bytes. If the method is
        called multiple times consecutively with different addresses, it is not
        efficient: It may do a word read and subsequently a block read that
        contains that word. (example: reading the serial number 4 bytes at 0x00
        is a word read, but reading the 5 bytes at 0x08 is a block read that
        happens to completely overlap the previous word read)

        :param address: the byte address to read at. This need not be
            word-aligned. The method will decide how to read.
        :param size: The number of bytes to read.
        """
        first_byte = addr
        last_byte = addr + size - 1

        read_addr = first_byte
        bytes_remaining = last_byte - read_addr + 1

        output = b''

        while bytes_remaining > 0:
            # The first byte read if we read a block
            block_read_addr = read_addr & (~0b11111)

            # the number of useful bytes returned from a block read
            block_read_bytes = min(last_byte, block_read_addr + 31) - max(read_addr, block_read_addr) + 1

            # The first byte read if we read a word
            word_read_addr = read_addr & (~0b11)

            # The number of useful bytes returned from a word read
            word_read_bytes = min(last_byte, word_read_addr + 3) - max(read_addr, word_read_addr) + 1

            # If doing a block read will get us more bytes we care about, do it
            if block_read_bytes > word_read_bytes:
                block = True
                read_bytes = block_read_bytes
                read_aligned_addr = block_read_addr
            else:
                block = False
                read_bytes = word_read_bytes
                read_aligned_addr = word_read_addr

            data = self.read(ECCX08_ZONE_CONFIG, read_aligned_addr, block)

            read_offset = read_addr - read_aligned_addr
            output += data[read_offset:read_offset + read_bytes]

            bytes_remaining -= read_bytes
            read_addr = read_offset + read_bytes

        return output

    def read_zone(self, zone, slot, block, offset, rx_size):
        assert (rx_size == 4) or (rx_size == 32)
        
        # The get address function checks the remaining variables
        addr = self.get_addr(zone, slot, block, offset)

        # If there are 32 bytes to write, then xor the bit into the mode
        if rx_size == 32:
            zone = zone | 0x80;

        # build a read command
        param1 = zone;
        param2 = addr;

        return self.send_cmd(ECCX08_CMD_READ, param1, param2, rx_size + 3, 1)
    
    def read(self, zone, addr, block=False):
        """
        Read data from one of the zones.
        """
        opcode = ECCX08_CMD_READ

        # a 1 in the high bit indicates an 8 word (32-byte) read. A 0 indicates a word read.
        block_bit = (1 << 7) if block else 0
        param1 = block_bit | zone

        # uint16_t: address to read from - This MUST be word aligned, note that
        # we truncate the two low bits. If the address is not word-aligned, it
        # will be!
        param2 = addr >> 2

        rx_size = 35 if block else 7

        # TODO: this and send_cmd() both assume this is a good reply.
        return self.send_cmd(opcode, param1, param2, rx_size, 1)

    def get_serial(self):
        """
        Fetch the serial number from the device.

        :return: a bytes() object containing the serial number.
        """
        # The serial number is contained at the block beginning at address 0x00.
        resp = self.read(ECCX08_ZONE_CONFIG, 0x00, True)

        serial_bytes = resp[0:4]
        serial_bytes += resp[8:13]

        return serial_bytes

    def get_revision(self):
        """
        Fetch the revision number from the device.
        """
        return self.read(ECCX08_ZONE_CONFIG, 0x04, False)

    def get_slot_config(self):
        """
        Get the configuration for all slots.
        """
        # slot config = bytes 20->51
        # we do this in two block reads: 0x00 - 0x1f, 0x20 - 0x40
        # two bytes per slot.
        data = self.read(ECCX08_ZONE_CONFIG, 0x00, True)
        data += self.read(ECCX08_ZONE_CONFIG, 0x20, True)

        # Starts at byte 20
        slot_config_offset = 0x14
        slots = []

        for slot_idx in range(16):
            start_idx = slot_config_offset + 2 * slot_idx
            slot_config_value = data[start_idx] + (data[start_idx + 1] << 8)

            slot_config = AtmelCryptoSlotConfig(slot_idx, slot_config_value)
            slots.append(slot_config)

            # TODO: this should be in a unit test.
            assert slot_config_value == slot_config.value

        return slots

    def get_key_config(self):
        """
        Get the KeyConfig for all slots.
        """
        # keyconfig data is bytes 96 - 127...
        # What's this? An actual block aligned value??? 32+32+32 = 96?

        data = self.read(ECCX08_ZONE_CONFIG, 0x60, True)

        slots = []

        for slot_idx in range(16):
            start_idx = 2 * slot_idx
            key_config_value = data[start_idx] + (data[start_idx + 1] << 8)

            key_config = AtmelCryptoKeyConfig(slot_idx, key_config_value)
            slots.append(key_config)

            assert key_config_value == key_config.value

        return slots

    def random(self, mode=0):
        """
        Implement the ECCx08 Random command.

        :param mode: Param1 of Random command.
        :return: 32 random bytes.
        """
        # Random number is 32 bytes => expect read 35
        return self.send_cmd(ECCX08_CMD_RANDOM, mode, 0x00, 35, 0.023)

    def mac(self, mode, key_id=0, challenge=None):
        """
        MAC command

        :param mode: May be either an integer with bits set as described in the
            datasheet or an instance of ``AtmelCryptoMACMode``. In either case,
            some validation is performed on the bits.

        :return: 32 bytes of SHA goodness
        """

        if isinstance(mode, AtmelCryptoMACMode):
            mode_value = mode.value
        else:
            mode_value = mode
            mode = AtmelCryptoMACMode(mode_value)

        if challenge is not None and mode['NotUseInputChallenge']:
            raise ValueError('Challenge parameter is ignored when Mode[0] is set.')

        elif challenge is None and not mode['NotUseInputChallenge']:
            raise ValueError('Challenge parameter is required when Mode[0] is not set.')

        elif len(challenge) != 32:
            raise ValueError('Challenge parameter must be 32 bytes.')

        elif mode['UseOTP88'] and mode['UseOTP64']:
            raise ValueError('Only one of Mode[4] or Mode[5] should be set.')

        elif mode['_Reserved1'] == 1 or mode['_Reserved2'] == 1:
            raise ValueError('_ReservedX bits must be 0.')

        # TODO: should we return bytes or int?
        return self.send_cmd(ECCX08_CMD_MAC, mode_value, key_id, 32, 11, challenge)


class BitField(object):
    """
    Consumes an integer and "parses" it into bit fields.

    For example, if we define::

        BIT_FIELDS = {
            'Foo' => (4, 3) # 3-bit field at offset 4..
        }

    Then ``foo['Foo'] == (value >> 4) & 0b111`` where ``value`` is the value
    passed into ``__init__()``.

    The value of the number represented by the bit fields can be obtained with
    the ``value`` property or via ``int()``: ``foo.value == int(foo)``.

    Bitwise operators work in the usual way, performing the bitwise operation
    on ``foo.value``::

        foo &= x # foo.value &= x
        foo |= x  # foo.value |= x
        foo >>= x # foo.value >>= x
        foo <<= x # foo.value <<= x

    In addition to providing access by field name, all bit fields can be
    accessed using integer keys or slices::

        # bits a - k (inclusive) of foo.value:
        foo[a:k] == (foo.value >> a) & ((2 ** (k - a + 1)) - 1))

        # bit a of foo.value
        foo[a] == foo[a:a] == (foo.value >> a) & 1

    Slice or integer key access is invalid if the size and offset of the slice
    does not exactly correspond to a defined field in ``BIT_FIELDS``. Note that
    unlike python's normal slice syntax, the start and end values *are*
    inclusive. This behavior matches the syntax used in the datasheet.
    """

    #: FieldName: (start_bit, number_of_bits). Override this!
    BIT_FIELDS = {}

    def __init__(self, value=None):
        self._fields = {}

        if value is not None:
            self._populate_fields(value)

    def _populate_fields(self, value):
        """
        :param value: An integer ;).
        """

        for bit_field in self.BIT_FIELDS:
            offset, size = self.BIT_FIELDS[bit_field]

            self._fields[bit_field] = ((value >> offset) & ((2 ** size) - 1), False)

    @property
    def value(self):
        """
        Return the Integer value suitable for writing back to the device. If
        the value was passed in with __init__ and not changed, this value
        should match.

        :return: An integer.
        """

        if set(self._fields.keys()) != set(self.BIT_FIELDS.keys()):
            raise ValueError('Cannot write back value unless all fields are populated.')

        bit_field_value = 0

        for bit_field in self.BIT_FIELDS:
            offset, size = self.BIT_FIELDS[bit_field]
            mask = (2 ** size) - 1
            value = self._fields[bit_field][0]

            bit_field_value += (mask & value) << offset

        return bit_field_value

    def __int__(self):
        """
        See :meth:`value`.
        """
        return self.value

    def __ior__(self, other):
        self._populate_fields(self.value | other)
        return self

    def __iand__(self, other):
        """
        self &= other
        """
        self._populate_fields(self.value & other)
        return self

    def __irshift__(self, other):
        """
        self >>= other
        """
        self._populate_fields(self.value >> other)
        return self

    def __ilshift__(self, other):
        """
        self <<= other
        """
        self._populate_fields(self.value << other)
        return self

    def __ixor__(self, other):
        """
        self ^= other
        """
        self._populate_fields(self.value ^ other)
        return self

    def __setitem__(self, field, value):
        """
        Set the value of the bit field ``field``.

        :param field: See :meth:`__getitem__`.
        :param value: The value of the field. This should be _only_ the value for the
            given bit field. If the size of the bit field is ``1``, a boolean
            may be passed.
        """
        field = self._resolve_field(field)

        size = self.BIT_FIELDS[field][1]

        if isinstance(value, bool):
            if size != 1:
                raise ValueError('bools may only be passed if size == 1')
            else:
                value = int(value)

        value_masked = value & ((2 ** size) - 1)

        # minimal validation: make sure the value passed in can fit in the desired field.
        if value_masked != value:
            raise ValueError('Invalid value 0x%x for %s: %s is a %d-bit field.' % (value, field, field, size))

        self._fields[field] = (value_masked, True)

    def _get_field_at(self, offset):
        for key in self.BIT_FIELDS:
            if offset == self.BIT_FIELDS[key][0]:
                return key, self.BIT_FIELDS[key][1]

        raise ValueError('No field at offset %d.' % offset)

    def _resolve_field(self, field):
        if isinstance(field, int):
            field_at, size_at = self._get_field_at(field)

            if size_at != 1:
                raise ValueError('%s is %d bits long; must be 1.' % field_at)
            else:
                return field_at

        elif isinstance(field, slice):
            field_at, size_at = self._get_field_at(field.start)

            if field.stop != field.start + size_at - 1:
                raise ValueError('[%d:%d] is a field; not [%d:%d]' % (field.start, field.start + size_at - 1, field.start, field.stop))

            else:
                return field_at

        elif field not in self.BIT_FIELDS:
            raise KeyError('Field %s does not exist.' % field)

        else:
            return field

    def __getitem__(self, field):
        """
        Return the bit field given by ``field``.

        For example, if we have a two byte integer, ``value``, and there is a 3
        bit field ``Foo`` at bits 4-6: ``foo['Foo'] == (value >> 4) & 0b111``.

        :param field: string, slice, or integer:
            * String: The name of the field as given in ``BIT_FIELDS``. If the
                bit field has name in the datasheet, this should match.
            * Integer: If an integer is passed, it is treated as an integer
                bit offset. If and only if the field at that field is 1 bit long.
            * Slice: self[a:k] returns an integer if and only if ``(a, k) in
                self.BIT_FIELDS.values()``. That is, bits ``a`` through ``k``
                inclusive represent exactly one bit field.
        :return: The value of the field - shfited and masked out.
        """
        field = self._resolve_field(field)

        if field not in self._fields:
            raise KeyError('Field %s not populated.' % field)

        return self._fields[field][0]

    def __str__(self):
        keys = sorted(self._fields.keys(), key=lambda key: self.BIT_FIELDS[key][0])

        fields = []
        for field in keys:
            offset, size = self.BIT_FIELDS[field]

            if size == 1:
                slice_str = '%d' % offset
            else:
                slice_str = '%d:%d' % (offset, offset + size - 1)

            fields.append('\tbit[%s](%s): %s' % (slice_str, field, bin(self._fields[field][0])))

        return '\n'.join(fields)


class AtmelCryptoSlotConfig(BitField):
    """
    Class to retrieve and manipulate the slot config values in the device's
    configuration section.

    :param slot: The number of the slot, 0->15
    :param slot_config: If passed, an integer representing the slot config for
        this slot.
    """

    # FieldName: (start_bit, number_of_bits)
    BIT_FIELDS = {
        'ReadKey': (0, 4),
        'NoMac': (4, 1),
        'LimitedUse': (5, 1),
        'EncryptRead': (6, 1),
        'IsSecret': (7, 1),
        'WriteKey': (8, 4),
        'WriteConfig': (12, 4),
    }

    def __init__(self, slot, slot_config=None):
        self._slot = slot
        super(AtmelCryptoSlotConfig, self).__init__(slot_config)

    @property
    def slot_idx(self):
        return self._slot

    def __str__(self):
        return 'SlotConfig[%d]\n' % self.slot_idx + super(AtmelCryptoSlotConfig, self).__str__()


class AtmelCryptoKeyConfig(BitField):
    """
    Class to retrieve and manipulate the KeyConfig values in the device's
    configuration section.

    :param slot: The number of the slot, 0->15
    :param key_config: If passed, an integer representing the key config for this slot.
    """

    BIT_FIELDS = {
        'Private': (0, 1),
        'PubInfo': (1, 1),
        'KeyType': (2, 3),
        'Lockable': (5, 1),
        'ReqRandom': (6, 1),
        'ReqAuth': (7, 1),
        'AuthKey': (8, 4),
        'IntrusionDisable': (12, 1),
        'RFU': (13, 1),
        'X509id': (14, 2)
    }

    def __init__(self, slot, key_config):
        self._slot = slot
        super(AtmelCryptoKeyConfig, self).__init__(key_config)

    @property
    def slot_idx(self):
        return self._slot

    def __str__(self):
        return 'KeyConfig[%d]\n' % self.slot_idx + super(AtmelCryptoKeyConfig, self).__str__()


class AtmelCryptoMACMode(BitField):
    """
    The bit fields of the ``MAC`` command's ``Mode`` parameter:

    * ``NotUseInputChallenge (Mode[0])``:
        * ``0``: The second 32 bytes of the SHA message are taken from the input
            ``challenge`` parameter.
        * ``1``: The second 32 bytes of the SHA message are taken from the value
            in ``TempKey``.
    * ``NotUseSlotData (Mode[1])``:
        * ``0``: The first 32 bytes of the SHA message are loaded from one of
            the data slots.
        * ``1``: The first 32 bytes are filled with ``TempKey``.
    * ``TempKeySourceFlag (Mode[2])``: If ``UseInputChallenge`` or ``UseSlotData`` is
        set, this bit must match the value in ```TempKey.SourceFlag``:
    * ``_Reserved1 (Mode[3])``: must be ``0``.
    * ``UseOTP88 (Mode[4])``: Set to ``1`` to include the first ``88`` OTP bits in the message.
    * ``UseOTP64 (Mode[5])``: Set to ``1`` to include the first ``64`` OTP bits in the
        mssage. Ignored if ``UseOTP88`` is set.
    * ``UseSN48 (Mode[6])``: Set to ``1`` to include the ``48`` bits ``SN[2:3]`` and
        ``SN[4:7]`` in the message.
    * ``_Reserved2 (Mode[7])``: must be ``0``.
    """

    BIT_FIELDS = {
        'NotUseInputChallenge': (0, 1),
        'NotUseSlotData': (1, 1),
        'TempKeySourceFlag': (2, 1),
        '_Reserved1': (3, 1),
        'UseOTP88': (4, 1),
        'UseOTP64': (5, 1),
        'UseSN48': (6, 1),
        '_Reserved2': (7, 1)
    }
