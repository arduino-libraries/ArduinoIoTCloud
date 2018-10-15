#ifndef ARDUINO_CLOUD_PROPERTY_CONTAINER_H_
#define ARDUINO_CLOUD_PROPERTY_CONTAINER_H_

#include "ArduinoCloudProperty.hpp"

#include "lib/tinycbor/cbor-lib.h"
#include "lib/LinkedList/LinkedList.h"

template <typename T>
class ArduinoCloudPropertyContainer {
public:

  void add(ArduinoCloudProperty<T> * property_obj);
  ArduinoCloudProperty<T> * operator [] (String const & name);
  int cntNumberOfPropertiesWhichShouldBeUpdated();
  void appendIfPropertyShouldBeUpdated(CborEncoder * arrayEncoder);

private:

  LinkedList<ArduinoCloudProperty<T> *> _list;

};

template <typename T>
void ArduinoCloudPropertyContainer<T>::add(ArduinoCloudProperty<T> * property_obj) {
  _list.add(property_obj);
}

template <typename T>
ArduinoCloudProperty<T> * ArduinoCloudPropertyContainer<T>::operator [] (String const & name) {
  for (int i = 0; i < _list.size(); i++) {
    ArduinoCloudProperty<T> * p = _list.get(i);
    if (p->name() == name) return p;
  }
  return 0;
}

template <typename T>
int ArduinoCloudPropertyContainer<T>::cntNumberOfPropertiesWhichShouldBeUpdated() {
  int should_be_updated_cnt = 0;

  for (int i = 0; i < _list.size(); i++) {
    ArduinoCloudProperty<T> * p = _list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      should_be_updated_cnt++;
    }
  }

  return should_be_updated_cnt;
}

template <typename T>
void ArduinoCloudPropertyContainer<T>::appendIfPropertyShouldBeUpdated(CborEncoder * arrayEncoder) {
  for (int i = 0; i < _list.size(); i++) {
    ArduinoCloudProperty<T> * p = _list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      p->append(arrayEncoder);
    }
  }
}

#endif /* ARDUINO_CLOUD_PROPERTY_CONTAINER_H_ */
