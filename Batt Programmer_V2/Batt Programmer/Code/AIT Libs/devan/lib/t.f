ee/utils/slot_data.py:189:                                          serial_number=b'\xFF' * 16,
ee/equipment/cy_bq28z610.py:167:                       'DFETF': 0x00020000,
ee/equipment/cy_bq28z610.py:168:                       'CFETF': 0x00010000}
ee/equipment/cy_bq28z610.py:502:            # print(F'{field_reg:x} {mask:x} {(field_reg & mask):x} {((field_reg & mask) != 0)}')
ee/equipment/cy_bq28z610.py:504:        # print(F'{flag_dict!r} {field_reg!r}')
ee/equipment/cy_bq28z610.py:549:                              'PF': 0x00001000,
ee/equipment/cy_bq28z610.py:651:                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
ee/equipment/cy_bq28z610.py:688:                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
ee/equipment/atcalib/hal/i2c/cypress.py:85:            raise IOError(F'Invalid status on wake got {data!r} expected {expected!r}')
ee/equipment/smbdat.py:612:                       'DFETF': 0x00020000,
ee/equipment/smbdat.py:613:                       'CFETF': 0x00010000}
ee/equipment/smbdat.py:701:#        assert (type(cmd) is bytes), F'cmd is type {type(cmd)}'
ee/equipment/smbdat.py:884:            # print(F'{field_reg:x} {mask:x} {(field_reg & mask):x} {((field_reg & mask) != 0)}')
ee/equipment/smbdat.py:886:        # print(F'{flag_dict!r} {field_reg!r}')
ee/equipment/smbdat.py:931:                              'PF': 0x00001000,
ee/equipment/smbdat.py:984:                        # print(F'Read {got:#06x} : {response[2:].hex()}')
ee/equipment/smbdat.py:987:                        # print(F'Read {got:#06x} : {response[2:(remaining_bytes+2)].hex()}')
ee/equipment/smbdat.py:1044:                #print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
ee/equipment/smbdat.py:1093:#                print(F'Unable to unpack buffer length expected({len(cls)}) got({len(buffer)})')
ee/equipment/smbdat.py:1360:#                struct.pack_into(F'{len(row_data)}s', buffer, addr - section['start'], row_data)
ee/equipment/smbdat.py:1375:                srec_data += F'S3{row_len:02X}{(section["start"]+offset):08X}{row_data.hex()}{crc:02X}\n'
ee/equipment/smbdat.py:1496:        #assert rom_version == 0x9101, F'Part {rom_version:#06x} is incompatible with programming'
ee/equipment/cy_smbdat.py:140:            print(F'IOTIMEOUT ({device_address:x})')
ee/equipment/cy_smbdat.py:205:        print(F'Reading {blk_len} bytes')
ee/equipment/cy_smbdat.py:207:        print(F'Recieved {len(response)} bytes as {response.hex()}')
ee/equipment/cy_smbdat.py:220:                    # print(F'{cb:02x} -> {pec:02x}')
ee/equipment/cy_smbdat.py:222:                # print(F'{(device_address << 1) | 1:02x} -> {pec:02x}')
ee/equipment/cy_smbdat.py:225:                    # print(F'{cb:02x} -> {pec:02x}')
ee/equipment/cy_smbdat.py:238:                print(F'Getting block size for command {command_code:02x} on device {device_address:02x}...')
ee/equipment/cy_smbdat.py:240:                print(F'Block len {blk_len}.')
