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

#ifndef CLOUDLIGHT_H_
#define CLOUDLIGHT_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../CloudBool.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/



class CloudLight : public CloudBool {
  private:
  public:
    operator bool() const                             {
      return _value;
    }
    CloudLight& operator=(bool v) {
      CloudBool::operator=(v);
      return *this;
    }
};


#endif /* CLOUDLIGHT_H_ */
