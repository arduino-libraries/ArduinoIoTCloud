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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "ArduinoCloudPropertyContainer.hpp"

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

bool ArduinoCloudPropertyContainer::isPropertyInContainer(Type const type, String const & name) {
  if      (type == Type::Bool  ) return isPropertyInList(_bool_property_list,   name);
  else if (type == Type::Int   ) return isPropertyInList(_int_property_list,    name);
  else if (type == Type::Float ) return isPropertyInList(_float_property_list,  name);
  else if (type == Type::String) return isPropertyInList(_string_property_list, name);
  else                           return false;
}

int ArduinoCloudPropertyContainer::getNumOfChangedProperties() {
  int num_changes_properties = 0;

  num_changes_properties += getNumOfChangedProperties(_bool_property_list  );
  num_changes_properties += getNumOfChangedProperties(_int_property_list   );
  num_changes_properties += getNumOfChangedProperties(_float_property_list );
  num_changes_properties += getNumOfChangedProperties(_string_property_list);

  return num_changes_properties;
}

void ArduinoCloudPropertyContainer::appendChangedProperties(CborEncoder * arrayEncoder) {
  appendChangedProperties<bool>  (_bool_property_list,   arrayEncoder);
  appendChangedProperties<int>   (_int_property_list,    arrayEncoder);
  appendChangedProperties<float> (_float_property_list,  arrayEncoder);
  appendChangedProperties<String>(_string_property_list, arrayEncoder);
}


int ArduinoCloudPropertyContainer::updateTimestampOnChangedProperties(unsigned long changeEventTime) {
  int num_changes_properties = 0;

  num_changes_properties += updateTimestampOnChangedProperties(_bool_property_list,   changeEventTime);
  num_changes_properties += updateTimestampOnChangedProperties(_int_property_list,    changeEventTime);
  num_changes_properties += updateTimestampOnChangedProperties(_float_property_list,  changeEventTime);
  num_changes_properties += updateTimestampOnChangedProperties(_string_property_list, changeEventTime);

  return num_changes_properties;
}
