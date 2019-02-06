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
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

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
void ArduinoCloudPropertyContainer::appendChangedProperties(LinkedList<ArduinoCloudProperty<T> *> & list, CborEncoder * arrayEncoder, CloudProtocol const cloud_protocol) {
  for (int i = 0; i < list.size(); i++) {
    ArduinoCloudProperty<T> * p = list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      p->append(arrayEncoder, cloud_protocol);
    }
  }
}
