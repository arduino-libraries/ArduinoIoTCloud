//
// This file is part of ArduinoCloudThing
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

#ifndef CLOUDWRAPPERSTRING_H_
#define CLOUDWRAPPERSTRING_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "AIoTC_Config.h"
#include "CloudWrapperBase.h"
#include <Arduino_DebugUtils.h>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class CloudWrapperString : public CloudWrapperBase {
  private:
    String  &_primitive_value,
            _cloud_value,
            _local_value;
  public:
    CloudWrapperString(String& v) :
      _primitive_value(v),
      _cloud_value(v),
      _local_value(v) {
    }
    virtual bool isDifferentFromCloud() {
      return _primitive_value != _cloud_value;
    }
    virtual void fromCloudToLocal() {
      _primitive_value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _primitive_value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      // check that the string fits mqtt tx buffer
      if(_name.length() > STRING_PROPERTY_MAX_SIZE) {
        DEBUG_WARNING(
          "[WARNING] %s:%s String property name exceedes transmit buffer size, unable to send property",
          __FILE__, __LINE__);

        /*
         * If your reached this line it means that you set a preperty name that exceeded the current maximum capabilities
         * of transmission buffer size. You can either change the buiffer size or the property name can be shortened.
         * Look below for raising the buffer size
         */

        return CborErrorOutOfMemory;
      } else if(_primitive_value.length() + _name.length() > STRING_PROPERTY_MAX_SIZE) {
        DEBUG_WARNING("[WARNING] %s:%s String property exceedes transmit buffer size", __FILE__, __LINE__);

        /*
         * If your reached this line it means that the value and the property name exceed the current maximum capabilities
         * of transmission buffer size. to fix this you can raise the size of the buffer.
         * you can raise the size of the buffer by setting #define MQTT_TX_BUFFER_SIZE <VALUE> at the beginning of the file
         */

        return appendAttribute("ERROR_PROPERTY_TOO_LONG", "", encoder);
      } else {
        return appendAttribute(_primitive_value, "", encoder);
      }
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value, "");
    }
    virtual bool isPrimitive() {
      return true;
    }
    virtual bool isChangedLocally() {
      return _primitive_value != _local_value;
    }
};


#endif /* CLOUDWRAPPERSTRING_H_ */
