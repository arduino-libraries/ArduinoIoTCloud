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

# Calculate length and CRC32
bin_data_len = len(bin_data)
bin_data_crc = crccheck.crc.Crc32.calc(bin_data)

# Write to outfile
out_file = open(ofile, "wb")
out_file.write((bin_data_len).to_bytes(4,byteorder='little'))
out_file.write((bin_data_crc).to_bytes(4,byteorder='little'))
out_file.write(bin_data)
out_file.close()
