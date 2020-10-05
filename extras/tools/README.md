Firmware Over-The-Air Tools
===========================

## How-to-OTA

Arduino IDE: `Sketch` -> `Export compiled Binary`
```bash
cp sketch.bin ~/Arduino/libraries/ArduinoIoTCloud/extras/tools/
cd ~/Arduino/libraries/ArduinoIoTCloud/extras/tools
./lzss.py --encode sketch.bin sketch.lzss
./bin2ota.py [MKR_WIFI_1010 | NANO_33_IOT] sketch.lzss sketch.ota
```

## `lzss.py`
This tool allows to compress a binary file using the LZSS algorithm.

### How-To-Use
* Encoding (Compressing)
```bash
./lzss.py --encode sketch.bin sketch.lzss
```
* Decoding (Extracting)
```bash
./lzss.py --decode sketch.lzss sketch.bin
```

## `bin2ota.py`
This tool can be used to extend (actually prefix) a binary generated with e.g. the Arduino IDE with the required length and crc values required to perform an OTA (Over-The-Air) update of the firmware.

### How-To-Use
```bash
./bin2ota.py [MKR_WIFI_1010 | NANO_33_IOT] sketch.lzss sketch.ota
```
#### `sketch.lzss`
```bash
 0   80602012 0a0cbe01 0094bfa2 bff7807c
16   bfdd00d9 655fd240 cfc11065 a071e0b2
...
```
#### `sketch.ota`
```bash
 0   3bca0100 7e1c3a2b 54804123 00000000
16   00000040 80602012 0a0cbe01 0094bfa2
...
```
* `length(sketch.lzss) = 3bca0100 -> 0x0001'CA3B`
* `CRC32(sketch.lzss + MAGIC NUMBER + VERSION) = 7e1c3a2b -> 0x2B3A'1C7E`
* `MAGIC NUMBER(MKR WIFI 1010) = 54804123 -> 0x2341'8054`
* `VERSION = 00000000 00000040 -> 0x40'00'00'00'00'00'00'00`
