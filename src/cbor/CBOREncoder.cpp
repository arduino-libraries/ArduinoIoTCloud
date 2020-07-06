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

#undef max
#undef min
#include <algorithm>

#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

CborError CBOREncoder::encode(PropertyContainer & property_container, uint8_t * data, size_t const size, int & bytes_encoded, bool lightPayload)
{
  CborEncoder encoder, arrayEncoder;

  cbor_encoder_init(&encoder, data, size, 0);

  CHECK_CBOR(cbor_encoder_create_array(&encoder, &arrayEncoder, CborIndefiniteLength));

  /* Check if backing storage and cloud has diverged
   * time interval may be elapsed or property may be changed
   * and if that's the case encode the property into the CBOR.
   */
  CborError error = CborNoError;
  int num_encoded_properties = 0;
  std::for_each(property_container.begin(),
                property_container.end(),
                [lightPayload, &arrayEncoder, &error, &num_encoded_properties](Property * p)
                {
                  if (p->shouldBeUpdated() && p->isReadableByCloud())
                  {
                    error = p->append(&arrayEncoder, lightPayload);
                    if(error == CborNoError)
                      num_encoded_properties++;
                    else
                      return;
                  }
                });
  if ((CborNoError != error) && 
      (CborErrorOutOfMemory != error))
    return error;

  CHECK_CBOR(cbor_encoder_close_container(&encoder, &arrayEncoder));

  if (num_encoded_properties > 0)
    bytes_encoded = cbor_encoder_get_buffer_size(&encoder, data);
  else
    bytes_encoded = 0;

  return CborNoError;
}
