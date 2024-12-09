#!/usr/bin/env python3

import sys
import crccheck

if len(sys.argv) != 4:
    print ("Usage: bin2ota.py BOARD sketch.bin sketch.ota")
    print ("  BOARD = [ MKR_WIFI_1010 | NANO_33_IOT | PORTENTA_H7_M7 | NANO_RP2040_CONNECT | NICLA_VISION | OPTA | GIGA | NANO_ESP32 | ESP32 | UNOR4WIFI]")
    sys.exit()

board = sys.argv[1]
ifile = sys.argv[2]
ofile = sys.argv[3]

# Read the binary file
in_file = open(ifile, "rb")
bin_data = bytearray(in_file.read())
in_file.close()

# Magic number (VID/PID)
if board == "MKR_WIFI_1010":
    magic_number = 0x23418054.to_bytes(4,byteorder='little')
elif board == "NANO_33_IOT":
    magic_number = 0x23418057.to_bytes(4,byteorder='little')
elif board == "PORTENTA_H7_M7":
    magic_number = 0x2341025B.to_bytes(4,byteorder='little')
elif board == "NANO_RP2040_CONNECT":
    magic_number = 0x2341005E.to_bytes(4,byteorder='little')
elif board == "NICLA_VISION":
    magic_number = 0x2341025F.to_bytes(4,byteorder='little')
elif board == "OPTA":
    magic_number = 0x23410064.to_bytes(4,byteorder='little')
elif board == "GIGA":
    magic_number = 0x23410266.to_bytes(4,byteorder='little')
elif board == "NANO_ESP32":
    magic_number = 0x23410070.to_bytes(4,byteorder='little')
# Magic number for all ESP32 boards not related to (VID/PID)
elif board == "ESP32":
    magic_number = 0x45535033.to_bytes(4,byteorder='little')
elif board == "UNOR4WIFI":
    magic_number = 0x23411002.to_bytes(4,byteorder='little')
elif board == "PORTENTA_C33":
    magic_number = 0x23410068.to_bytes(4,byteorder='little')
else:
    print ("Error,", board, "is not a supported board type")
    sys.exit()

# Version field (byte array of size 8) - all 0 except the compression flag set.
version = bytearray([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40])

# Prepend magic number and version field to payload
bin_data_complete = magic_number + version + bin_data

# Calculate length and CRC32
bin_data_len = len(bin_data_complete)
bin_data_crc = crccheck.crc.Crc32.calc(bin_data_complete)

# Write to outfile
out_file = open(ofile, "wb")
out_file.write((bin_data_len).to_bytes(4,byteorder='little'))
out_file.write((bin_data_crc).to_bytes(4,byteorder='little'))
out_file.write(bin_data_complete)
out_file.close()
