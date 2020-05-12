`bin2ota.py`
============
This tool can be used to extend (actually prefix) a binary generated with e.g. the Arduino IDE with the required length and crc values required to perform an OTA (Over-The-Air) update of the firmware.

### How-To-Use
```bash
./bin2ota.py sketch.bin sketch-ota.bin
```
#### `sketch.bin`
```bash
0000000   8000 2000 749D 0000 7485 0000 7485 0000
0000010   0000 0000 0000 0000 0000 0000 0000 0000
0000020   0000 0000 0000 0000 0000 0000 7485 0000
0000030   0000 0000 0000 0000 7485 0000 74F1 0000
...
```
* `length(sketch.bin) = 0x0003'A5E0`
* `CRC32(sketch.bin)  = 0xA9D1'265B`

#### `sketch-ota.bin`
```bash
0000000   A5E0 0003 265B A9D1 8000 2000 749D 0000
0000010   7485 0000 7485 0000 0000 0000 0000 0000
0000020   0000 0000 0000 0000 0000 0000 0000 0000
0000030   0000 0000 7485 0000 0000 0000 0000 0000
...
