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

#ifndef ARDUINO_CBOR_CBOR_ENCODER_H_
#define ARDUINO_CBOR_CBOR_ENCODER_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "../property/PropertyContainer.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class CBOREncoder
{

public:

    /* encode return > 0 if a property has changed and encodes the changed properties in CBOR format into the provided buffer */
    /* if lightPayload is true the integer identifier of the property will be encoded in the message instead of the property name in order to reduce the size of the message payload*/
    static CborError encode(PropertyContainer & property_container, uint8_t * data, size_t const size, int & bytes_encoded, bool lightPayload = false);

private:

  CBOREncoder() { }
  CBOREncoder(CborEncoder const &) { }

};

#endif /* ARDUINO_CBOR_CBOR_ENCODER_H_ */
