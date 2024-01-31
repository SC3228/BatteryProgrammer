self._i2c_addr = 97
ioctl 3 1795 97
write data =  b'\x00'
opcode,param1,param2 0x2 0x82 0x40
op_data b'\x02\x82@\x00'
tx_buf =  b'\x03\x07\x02\x82@\x00\t\xa4'
write data =  b'\x03\x07\x02\x82@\x00\t\xa4'
data read b'#\x03\xffZEBRA\xa92014P1089503-0030A23418B\xf2\xc7'
write data =  b'\x01'
resp_bytes b'#\x03\xffZEBRA\xa92014P1089503-0030A23418B\xf2\xc7'
write data =  b'\x00'
opcode,param1,param2 0x2 0x82 0x140
op_data b'\x02\x82@\x01'
tx_buf =  b"\x03\x07\x02\x82@\x01\n'"
write data =  b"\x03\x07\x02\x82@\x01\n'"
write data =  b'\x01'


        error_count = 0
        while True:
            if error_count > 3:
                return  None            #abort, too many errors
            try:
                os.write(self._i2c_dev, data)
            except:
                error_count += 1
                continue
            break
