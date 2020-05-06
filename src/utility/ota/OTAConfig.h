/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

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

#ifndef ARDUINO_OTA_CONFIG_H_
#define ARDUINO_OTA_CONFIG_H_

/******************************************************************************
 * DEFINE
 ******************************************************************************/

#ifndef OTA_STORAGE_MKRMEM
  #define OTA_STORAGE_MKRMEM      (1)
#endif

#if !defined(ARDUINO_SAMD_MKR1000) && !defined(ARDUINO_SAMD_MKRWIFI1010) && !defined(ARDUINO_SAMD_MKRGSM1400) && !defined(ARDUINO_SAMD_MKRNB1500)
  #define OTA_STORAGE_MKRMEM      (0)
#endif

#endif /* ARDUINO_OTA_CONFIG_H_ */
