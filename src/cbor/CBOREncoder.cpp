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
#include <iterator>

#include "lib/tinycbor/cbor-lib.h"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

CborError CBOREncoder::encode(PropertyContainer & property_container, uint8_t * data, size_t const size, int & bytes_encoded, unsigned int & current_property_index, bool lightPayload)
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
  int num_checked_properties = 0;
  static int encoded_properties_message_limit = CBOR_ENCODER_NO_PROPERTIES_LIMIT;

  if(current_property_index >= property_container.size())
    current_property_index = 0;

  PropertyContainer::iterator iter = property_container.begin();
  std::advance(iter, current_property_index);

  std::for_each(iter,
                property_container.end(),
                [lightPayload, &arrayEncoder, &error, &num_encoded_properties, &num_checked_properties](Property * p)
                {
                  bool maximum_number_of_properties_reached = (num_encoded_properties >= encoded_properties_message_limit) && (encoded_properties_message_limit != CBOR_ENCODER_NO_PROPERTIES_LIMIT);
                  bool cbor_encoder_error = (error != CborNoError);
                  if((!cbor_encoder_error) && (!maximum_number_of_properties_reached))
                  {
                    if (p->shouldBeUpdated() && p->isReadableByCloud())
                    {
                      error = p->append(&arrayEncoder, lightPayload);
                      if(error == CborNoError)
                        num_encoded_properties++;
                      else
                        return;
                    }
                    num_checked_properties++;
                  }
                });

  if ((CborNoError != error) && (CborErrorOutOfMemory != error))
  {
    /* Trim the number of properties to be included in the next message to avoid multivalue property split */
    encoded_properties_message_limit = num_encoded_properties;
    return error;
  }

  error = cbor_encoder_close_container(&encoder, &arrayEncoder);
  if (CborNoError != error)
  {
    /* Trim the number of properties to be included in the next message to avoid error closing container */
    encoded_properties_message_limit = num_encoded_properties - 1;
    return error;
  }

  /* Restore property message limit to CBOR_ENCODER_NO_PROPERTIES_LIMIT */
  encoded_properties_message_limit = CBOR_ENCODER_NO_PROPERTIES_LIMIT;

  /* The append process has been successful, so we don't need to terty to send this properties set. Cleanup _has_been_appended_but_not_sended flag */
  iter = property_container.begin();
  std::advance(iter, current_property_index);
  int num_appended_properties = 0;
  std::for_each(iter,
                property_container.end(),
                [&num_appended_properties, &num_checked_properties](Property * p)
                {
                  if (num_appended_properties < num_checked_properties)
                  {
                    p->appendCompleted();
                    num_appended_properties++;
                  }
                });

  /* Advance property index for the nex message */
  current_property_index += num_checked_properties;

  if (num_encoded_properties > 0)
    bytes_encoded = cbor_encoder_get_buffer_size(&encoder, data);
  else
    bytes_encoded = 0;

  return CborNoError;
}
