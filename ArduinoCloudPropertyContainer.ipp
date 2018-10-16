
template <typename T>
bool ArduinoCloudPropertyContainer::isPropertyInList(LinkedList<ArduinoCloudProperty<T> *> & list, String const & name) {
  for (int i = 0; i < list.size(); i++) {
    ArduinoCloudProperty<T> * p = list.get(i);
    if (p->name() == name) return true;
  }
  return false;
}

template <typename T>
ArduinoCloudProperty<T> * ArduinoCloudPropertyContainer::getProperty(LinkedList<ArduinoCloudProperty<T> *> & list, String const & name) {
  for (int i = 0; i < list.size(); i++) {
    ArduinoCloudProperty<T> * p = list.get(i);
    if (p->name() == name) return p;
  }
  return 0;
}

template <typename T>
int ArduinoCloudPropertyContainer::getNumOfChangedProperties(LinkedList<ArduinoCloudProperty<T> *> & list) {
  int num_changes_properties = 0;

  for (int i = 0; i < list.size(); i++) {
    ArduinoCloudProperty<T> * p = list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      num_changes_properties++;
    }
  }

  return num_changes_properties;
}

template <typename T>
void ArduinoCloudPropertyContainer::appendChangedProperties(LinkedList<ArduinoCloudProperty<T> *> & list, CborEncoder * arrayEncoder) {
  for (int i = 0; i < list.size(); i++) {
    ArduinoCloudProperty<T> * p = list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      p->append(arrayEncoder);
    }
  }
}
