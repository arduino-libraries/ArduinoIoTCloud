#!/usr/bin/python3

import sys
import crccheck

if len(sys.argv) != 3:
    print ("Usage: bin2ota.py sketch.bin sketch.ota")
    sys.exit()

ifile = sys.argv[1]
ofile = sys.argv[2]

# Read the binary file
in_file = open(ifile, "rb")
bin_data = bytearray(in_file.read())
in_file.close()

# Magic number (VID/PID), hard coded for MKR VIDOR 4000 right now
magic_number = bytearray([0x23, 0x41, 0x80, 0x54])
print(magic_number)

# Version field (byte array of size 8)
version = bytearray(8)
print(version)

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
