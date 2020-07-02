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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "CBOREncoder.h"

#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int CBOREncoder::encode(PropertyContainer & property_container, uint8_t * data, size_t const size, bool lightPayload)
{
  CborEncoder encoder, arrayEncoder;

  cbor_encoder_init(&encoder, data, size, 0);

  if (cbor_encoder_create_array(&encoder, &arrayEncoder, CborIndefiniteLength) != CborNoError)
    return -1;

  /* Check if backing storage and cloud has diverged
   * time interval may be elapsed or property may be changed
   * and if that's the case encode the property into the CBOR.
   */
  if (appendChangedProperties(property_container, &arrayEncoder, lightPayload) < 1)
    return -1;

  if (cbor_encoder_close_container(&encoder, &arrayEncoder) != CborNoError)
    return -1;

  int const bytes_encoded = cbor_encoder_get_buffer_size(&encoder, data);
  return bytes_encoded;
}
