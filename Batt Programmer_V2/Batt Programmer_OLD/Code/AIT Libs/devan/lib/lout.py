from cryptoauthlib import *

ATCA_SUCCESS = 0x00
revision = bytearray(4)
randomnum = bytearray(32)

# Locate and load the compiled library
load_cryptoauthlib()

assert ATCA_SUCCESS == atcab_init(cfg_ateccx08a_kithid_default())

assert ATCA_SUCCESS == atcab_info(revision)
print(''.join(['%02X ' % x for x in revision]))

assert ATCA_SUCCESS == atcab_random(randomnum)
print(''.join(['%02X ' % x for x in randomnum]))
