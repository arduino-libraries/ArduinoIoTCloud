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

#ifndef ARDUINO_ECCX08_CONFIG_H
#define ARDUINO_ECCX08_CONFIG_H


#if defined __has_include
  /* Disable ECCX08 SSC if not explicitly included */
  #if !__has_include (<utility/ECCX08SelfSignedCert.h>)
     #define ECCX08_DISABLE_ASN1
     #define ECCX08_DISABLE_SSC
  #endif
  /* Disable ECCX08 JWS support if not explicitly included */
  #if !__has_include (<utility/ECCX08JWS.h>)
    #define ECCX08_DISABLE_JWS
  #endif
  /* Disable ECCX08 CSR support if not explicitly included */
  #if !__has_include (<utility/ECCX08CSR.h>)
    #define ECCX08_DISABLE_CSR
  #endif
   /* Disable ECCX08 PEM support if not explicitly included */
  #if !__has_include (<utility/PEMUtils.h>)
    #define ECCX08_DISABLE_PEM
  #endif
#endif

#endif /* ARDUINO_ECCX08_CONFIG_H */
