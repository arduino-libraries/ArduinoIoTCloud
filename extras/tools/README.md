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

## `bin2ota.py`
This tool can be used to extend (actually prefix) a binary generated with e.g. the Arduino IDE with the required length and crc values required to perform an OTA (Over-The-Air) update of the firmware.

### How-To-Use
```bash
./bin2ota.py [MKR_WIFI_1010 | NANO_33_IOT] sketch.bin sketch.ota
```
#### `sketch.bin`
```bash
0000000   8000 2000 749D 0000 7485 0000 7485 0000
0000030   0000 0000 0000 0000 7485 0000 74F1 0000
...
```
* `length(sketch.bin) = 0x0003'A5E0`
* `CRC32(sketch.bin)  = 0xA9D1'265B`
* `MAGIC NUMBER(MKR WIFI 1010)  = 0x2341'8054`
* `VERSION = 0x0000'0000'0000'0000`

#### `sketch.ota`
```bash
0000000   A5E0 0003 265B A9D1 2341 8054 0000 0000
0000010   0000 0000 8000 2000 749D 0000 7485 0000
...
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