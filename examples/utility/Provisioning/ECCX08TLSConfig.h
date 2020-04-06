/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef _ECCX08_TLS_CONFIG_H_
#define _ECCX08_TLS_CONFIG_H_

const byte DEFAULT_ECCX08_TLS_CONFIG[128] = {
  // Read only - start
  // SN[0:3]
  0x01, 0x23, 0x00, 0x00,
  // RevNum
  0x00, 0x00, 0x50, 0x00,
  // SN[4:8]
  0x00, 0x00, 0x00, 0x00, 0x00,
  // Reserved
  0xC0,
  // I2C_Enable
  0x71,
  // Reserved
  0x00,
  // Read only - end
  // I2C_Address
  0xC0,
  // Reserved
  0x00,
  // OTPmode
  0x55,
  // ChipMode
  0x00,
  // SlotConfig
  0x83, 0x20, // External Signatures | Internal Signatures | IsSecret | Write Configure Never, Default: 0x83, 0x20,
  0x87, 0x20, // External Signatures | Internal Signatures | ECDH | IsSecret | Write Configure Never, Default: 0x87, 0x20,
  0x87, 0x20, // External Signatures | Internal Signatures | ECDH | IsSecret | Write Configure Never, Default: 0x8F, 0x20,
  0x87, 0x2F, // External Signatures | Internal Signatures | ECDH | IsSecret | WriteKey all slots | Write Configure Never, Default: 0xC4, 0x8F,
  0x87, 0x2F, // External Signatures | Internal Signatures | ECDH | IsSecret | WriteKey all slots | Write Configure Never, Default: 0x8F, 0x8F,
  0x8F, 0x8F,
  0x9F, 0x8F,
  0xAF, 0x8F,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0xAF, 0x8F,
  // Counter[0]
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
  // Counter[1]
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
  // LastKeyUse
  0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF,
  // Write via commands only - start
  // UserExtra
  0x00,
  // Selector
  0x00,
  // LockValue
  0x55,
  // LockConfig
  0x55,
  // SlotLocked
  0xFF, 0xFF,
  // Write via commands only - end
  // RFU
  0x00, 0x00,
  // X509format
  0x00, 0x00, 0x00, 0x00,
  // KeyConfig
  0x33, 0x00, // Private | Public | P256 NIST ECC key, Default: 0x33, 0x00,
  0x33, 0x00, // Private | Public | P256 NIST ECC key, Default: 0x33, 0x00,
  0x33, 0x00, // Private | Public | P256 NIST ECC key, Default: 0x33, 0x00,
  0x33, 0x00, // Private | Public | P256 NIST ECC key, Default: 0x1C, 0x00,
  0x33, 0x00, // Private | Public | P256 NIST ECC key, Default: 0x1C, 0x00,
  0x1C, 0x00,
  0x1C, 0x00,
  0x1C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x3C, 0x00,
  0x1C, 0x00
};

#endif /* _ECCX08_TLS_CONFIG_H_ */
