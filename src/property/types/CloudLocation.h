/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
