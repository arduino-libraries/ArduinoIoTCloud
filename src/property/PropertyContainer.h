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
   DECLARATION OF getTime
 ******************************************************************************/

#ifdef HAS_LORA
static unsigned long constexpr getTime() { return 0; }
#else
extern "C" unsigned long getTime();
#endif

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

typedef std::list<Property *> PropertyContainer;

/******************************************************************************
   FUNCTION DECLARATION
 ******************************************************************************/

Property & addPropertyToContainer(PropertyContainer & prop_cont,
                                  Property & property,
                                  String const & name,
                                  Permission const permission,
                                  int propertyIdentifier = -1,
                                  GetTimeCallbackFunc func = getTime);

  
Property * getProperty(PropertyContainer & prop_cont, String const & name);
Property * getProperty(PropertyContainer & prop_cont, int const identifier);


int appendChangedProperties(PropertyContainer & prop_cont, CborEncoder * arrayEncoder, bool lightPayload);
void updateTimestampOnLocallyChangedProperties(PropertyContainer & prop_cont);
void requestUpdateForAllProperties(PropertyContainer & prop_cont);

void addProperty(PropertyContainer & prop_cont, Property * property_obj, int propertyIdentifier);

#endif /* ARDUINO_PROPERTY_CONTAINER_H_ */