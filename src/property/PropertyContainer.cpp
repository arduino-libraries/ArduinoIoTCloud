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
   INCLUDE
 ******************************************************************************/

#include "PropertyContainer.h"

#include <algorithm>

#include "types/CloudWrapperBase.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

PropertyContainer::PropertyContainer()
: _numPrimitivesProperties{0}
, _get_time_func{nullptr}
{

}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void PropertyContainer::begin(GetTimeCallbackFunc func)
{
  _get_time_func = func;
}

Property & PropertyContainer::addPropertyReal(Property & property, String const & name, Permission const permission, int propertyIdentifier)
{
  /* Check whether or not the property already has been added to the container */
  Property * p = getProperty(name);
  if(p != nullptr) return (*p);

  /* Initialize property and add it to the container */
  property.init(name, permission, _get_time_func);

  if (property.isPrimitive()) { _numPrimitivesProperties++; }
  addProperty(&property, propertyIdentifier);
  return property;
}


Property * PropertyContainer::getProperty(String const & name)
{
  std::list<Property *>::iterator iter;

  iter = std::find_if(_property_list.begin(),
                      _property_list.end(),
                      [name](Property * p) -> bool
                      {
                        return (p->name() == name);
                      });

  if (iter == _property_list.end())
    return nullptr;
  else
    return (*iter);
}

Property * PropertyContainer::getProperty(int const identifier)
{
  std::list<Property *>::iterator iter;

  iter = std::find_if(_property_list.begin(),
                      _property_list.end(),
                      [identifier](Property * p) -> bool
                      {
                        return (p->identifier() == identifier);
                      });

  if (iter == _property_list.end())
    return nullptr;
  else
    return (*iter);
}

int PropertyContainer::appendChangedProperties(CborEncoder * arrayEncoder, bool lightPayload)
{
  int appendedProperties = 0;
  std::for_each(_property_list.begin(),
                _property_list.end(),
                [arrayEncoder, lightPayload, &appendedProperties](Property * p)
                {
                  if (p->shouldBeUpdated() && p->isReadableByCloud())
                  {
                    p->append(arrayEncoder, lightPayload);
                    appendedProperties++;
                  }
                });
  return appendedProperties;
}

void PropertyContainer::requestUpdateForAllProperties()
{
  std::for_each(_property_list.begin(),
                _property_list.end(),
                [](Property * p)
                {
                  p->requestUpdate();
                });
}

void PropertyContainer::updateTimestampOnLocallyChangedProperties()
{
  /* This function updates the timestamps on the primitive properties 
   * that have been modified locally since last cloud synchronization
   */
  if (_numPrimitivesProperties > 0)
  {
    std::for_each(_property_list.begin(),
                  _property_list.end(),
                  [](Property * p)
                  {
                    CloudWrapperBase * pbase = reinterpret_cast<CloudWrapperBase *>(p);
                    if (pbase->isPrimitive() && pbase->isChangedLocally() && pbase->isReadableByCloud())
                    {
                      p->updateLocalTimestamp();
                    }
                  });
  }
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void PropertyContainer::addProperty(Property * property_obj, int propertyIdentifier)
{
  if (propertyIdentifier != -1)
  {
    property_obj->setIdentifier(propertyIdentifier);
  }
  /* If property identifier is -1, an incremental value will be assigned as identifier. */
  else
  {
    property_obj->setIdentifier(_property_list.size() + 1); /* This is in order to stay compatible to the old system of first increasing _numProperties and then assigning it here. */
  }
  _property_list.push_back(property_obj);
}
