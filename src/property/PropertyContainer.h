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

#ifndef ARDUINO_PROPERTY_CONTAINER_H_
#define ARDUINO_PROPERTY_CONTAINER_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "Property.h"

#undef max
#undef min
#include <list>

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class PropertyContainer
{

public:

  PropertyContainer();

  
  void begin(GetTimeCallbackFunc func);


  Property & addPropertyReal(Property & property, String const & name, Permission const permission, int propertyIdentifier = -1);

  
  Property * getProperty          (String const & name);
  Property * getProperty          (int const identifier);


  int appendChangedProperties(CborEncoder * arrayEncoder, bool lightPayload);
  void updateTimestampOnLocallyChangedProperties();
  void requestUpdateForAllProperties();



private:

  int _numProperties;
  int _numPrimitivesProperties;
  GetTimeCallbackFunc _get_time_func;
  std::list<Property *> _property_list;

  void addProperty(Property * property_obj, int propertyIdentifier);

};

#endif /* ARDUINO_PROPERTY_CONTAINER_H_ */