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

#ifndef CLOUDLOCATION_H_
#define CLOUDLOCATION_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <math.h>
#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/



class Location {
  public:
    float lat,
          lon;
    Location(float lat, float lon) : lat(lat), lon(lon) {}
    Location& operator=(Location& aLocation) {
      lat = aLocation.lat;
      lon = aLocation.lon;
      return *this;
    }
    Location operator-(Location& aLocation) {
      return Location(lat - aLocation.lat, lon - aLocation.lon);
    }
    bool operator==(Location& aLocation) {
      return lat == aLocation.lat && lon == aLocation.lon;
    }
    bool operator!=(Location& aLocation) {
      return !(operator==(aLocation));
    }
    static float distance(Location& loc1, Location& loc2) {
      return sqrt(pow(loc1.lat - loc2.lat, 2) + pow(loc1.lon - loc2.lon, 2));
    }
};

class CloudLocation : public Property {
  private:
    Location _value,
             _cloud_value;
  public:
    CloudLocation() : _value(0, 0), _cloud_value(0, 0) {}
    CloudLocation(float lat, float lon) : _value(lat, lon), _cloud_value(lat, lon) {}
    virtual bool isDifferentFromCloud() {
      float const distance = Location::distance(_value, _cloud_value);
      return _value != _cloud_value && (abs(distance) >= Property::_min_delta_property);
    }

    CloudLocation& operator=(Location aLocation) {
      _value.lat = aLocation.lat;
      _value.lon = aLocation.lon;
      updateLocalTimestamp();
      return *this;
    }

    Location getCloudValue() {
      return _cloud_value;
    }

    Location getValue() {
      return _value;
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      CHECK_CBOR_MULTI(appendAttribute(_value.lat, "lat", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.lon, "lon", encoder));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.lat, "lat");
      setAttribute(_cloud_value.lon, "lon");
    }
};

#endif /* CLOUDLOCATION_H_ */
