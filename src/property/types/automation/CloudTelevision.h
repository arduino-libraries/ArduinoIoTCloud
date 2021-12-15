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

#ifndef CLOUDTELEVISION_H_
#define CLOUDTELEVISION_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include "../../Property.h"

/******************************************************************************
   ENUM
 ******************************************************************************/
enum class PlaybackCommands : int {
  FastForward   = 0,
  Next          = 1,
  Pause         = 2,
  Play          = 3,
  Previous      = 4,
  Rewind        = 5,
  StartOver     = 6,
  Stop          = 7,
  None          = 255
};
enum class InputValue : int {
  AUX1          = 0,
  AUX2          = 1,
  AUX3          = 2,
  AUX4          = 3,
  AUX5          = 4,
  AUX6          = 5,
  AUX7          = 6,
  BLURAY        = 7,
  CABLE         = 8,
  CD            = 9,
  COAX1         = 10,
  COAX2         = 11,
  COMPOSITE1    = 12,
  DVD           = 13,
  GAME          = 14,
  HDRADIO       = 15,
  HDMI1         = 16,
  HDMI2         = 17,
  HDMI3         = 18,
  HDMI4         = 19,
  HDMI5         = 20,
  HDMI6         = 21,
  HDMI7         = 22,
  HDMI8         = 23,
  HDMI9         = 24,
  HDMI10        = 25,
  HDMIARC       = 26,
  INPUT1        = 27,
  INPUT2        = 28,
  INPUT3        = 29,
  INPUT4        = 30,
  INPUT5        = 31,
  INPUT6        = 32,
  INPUT7        = 33,
  INPUT8        = 34,
  INPUT9        = 35,
  INPUT10       = 36,
  IPOD          = 37,
  LINE1         = 38,
  LINE2         = 39,
  LINE3         = 40,
  LINE4         = 41,
  LINE5         = 42,
  LINE6         = 43,
  LINE7         = 44,
  MEDIAPLAYER   = 45,
  OPTICAL1      = 46,
  OPTICAL2      = 47,
  PHONO         = 48,
  PLAYSTATION   = 49,
  PLAYSTATION3  = 50,
  PLAYSTATION4  = 51,
  SATELLITE     = 52,
  SMARTCAST     = 53,
  TUNER         = 54,
  TV            = 55,
  USBDAC        = 56,
  VIDEO1        = 57,
  VIDEO2        = 58,
  VIDEO3        = 59,
  XBOX          = 60
};

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class Television {
  public:
    bool                swi;
    int                 vol;
    bool                mut;
    PlaybackCommands    pbc;
    InputValue          inp;
    int                 cha;


    Television(bool const swi, int const vol, bool const mut, PlaybackCommands const pbc, InputValue const inp, int const cha): swi(swi), vol(vol), mut(mut), pbc(pbc), inp(inp), cha(cha) {
    }

    bool operator==(Television const & aTV) {
      return
        aTV.swi == swi &&
        aTV.vol == vol &&
        aTV.mut == mut &&
        aTV.pbc == pbc &&
        aTV.inp == inp &&
        aTV.cha == cha;
    }

    bool operator!=(Television const & aTV) {
      return !(operator==(aTV));
    }

};

class CloudTelevision : public Property {
  private:
    Television _value,
               _cloud_value;
  public:
    CloudTelevision() : _value(false, 0, false, PlaybackCommands::None, InputValue::TV, 0), _cloud_value(false, 0, false, PlaybackCommands::None, InputValue::TV, 0) {}
    CloudTelevision(bool const swi, int const vol, bool const mut, PlaybackCommands const pbc, InputValue const inp, int const cha) : _value(swi, vol, mut, pbc, inp, cha), _cloud_value(swi, vol, mut, pbc, inp, cha) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudTelevision& operator=(Television const aTV) {
      _value.swi = aTV.swi;
      _value.vol = aTV.vol;
      _value.mut = aTV.mut;
      _value.pbc = aTV.pbc;
      _value.inp = aTV.inp;
      _value.cha =  aTV.cha;
      updateLocalTimestamp();
      return *this;
    }

    Television getCloudValue() {
      return _cloud_value;
    }

    Television getValue() {
      return _value;
    }

    void setSwitch(bool const swi) {
      _value.swi = swi;
      updateLocalTimestamp();
    }

    bool getSwitch() {
      return _value.swi;
    }

    void setVolume(uint8_t const vol) {
      _value.vol = vol;
      updateLocalTimestamp();
    }

    uint8_t getVolume() {
      return _value.vol;
    }

    void setMute(bool const mut) {
      _value.mut = mut;
      updateLocalTimestamp();
    }

    bool getMute() {
      return _value.mut;
    }

    PlaybackCommands getPlaybackCommand() {
      return _value.pbc;
    }

    InputValue getInputValue() {
      return _value.inp;
    }

    uint16_t getChannel() {
      return _value.cha;
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud() {
      CHECK_CBOR_MULTI(appendAttribute(_value.swi));
      CHECK_CBOR_MULTI(appendAttribute(_value.vol));
      CHECK_CBOR_MULTI(appendAttribute(_value.mut));
      CHECK_CBOR_MULTI(appendAttribute((int)_value.pbc));
      CHECK_CBOR_MULTI(appendAttribute((int)_value.inp));
      CHECK_CBOR_MULTI(appendAttribute(_value.cha));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.swi);
      setAttribute(_cloud_value.vol);
      setAttribute(_cloud_value.mut);
      setAttribute((int&)_cloud_value.pbc);
      setAttribute((int&)_cloud_value.inp);
      setAttribute(_cloud_value.cha);
    }
};

#endif /* CLOUDTELEVISION_H_ */
