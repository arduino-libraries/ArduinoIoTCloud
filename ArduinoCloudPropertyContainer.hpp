#ifndef ARDUINO_CLOUD_PROPERTY_CONTAINER_NEW_H_
#define ARDUINO_CLOUD_PROPERTY_CONTAINER_NEW_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "ArduinoCloudProperty.hpp"

#include "lib/tinycbor/cbor-lib.h"
#include "lib/LinkedList/LinkedList.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoCloudPropertyContainer {
public:

  bool isPropertyInContainer    (Type const type, String const & name);
  int  getNumOfChangedProperties();
  void appendChangedProperties  (CborEncoder * arrayEncoder, CloudProtocol const cloud_protocol);

  inline ArduinoCloudProperty<bool>   * getPropertyBool  (String const & name) { return getProperty(_bool_property_list,   name); }
  inline ArduinoCloudProperty<int>    * getPropertyInt   (String const & name) { return getProperty(_int_property_list,    name); }
  inline ArduinoCloudProperty<float>  * getPropertyFloat (String const & name) { return getProperty(_float_property_list,  name); }
  inline ArduinoCloudProperty<String> * getPropertyString(String const & name) { return getProperty(_string_property_list, name); }

  inline void addProperty(ArduinoCloudProperty<bool>   * property_obj) { _bool_property_list.add  (property_obj); }
  inline void addProperty(ArduinoCloudProperty<int>    * property_obj) { _int_property_list.add   (property_obj); }
  inline void addProperty(ArduinoCloudProperty<float>  * property_obj) { _float_property_list.add (property_obj); }
  inline void addProperty(ArduinoCloudProperty<String> * property_obj) { _string_property_list.add(property_obj); }

private:

  LinkedList<ArduinoCloudProperty<bool>   *> _bool_property_list;
  LinkedList<ArduinoCloudProperty<int>    *> _int_property_list;
  LinkedList<ArduinoCloudProperty<float>  *> _float_property_list;
  LinkedList<ArduinoCloudProperty<String> *> _string_property_list;

  template <typename T>
  bool isPropertyInList(LinkedList<ArduinoCloudProperty<T> *> & list, String const & name);

  template <typename T>
  ArduinoCloudProperty<T> * getProperty(LinkedList<ArduinoCloudProperty<T> *> & list, String const & name);

  template <typename T>
  int getNumOfChangedProperties(LinkedList<ArduinoCloudProperty<T> *> & list);

  template <typename T>
  void appendChangedProperties(LinkedList<ArduinoCloudProperty<T> *> & list, CborEncoder * arrayEncoder, CloudProtocol const cloud_protocol);

};

/******************************************************************************
 * TEMPLATE IMPLEMENTATION
 ******************************************************************************/

#include "ArduinoCloudPropertyContainer.ipp"

#endif /* ARDUINO_CLOUD_PROPERTY_CONTAINER_NEW_H_ */
