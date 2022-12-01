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

#ifndef CLOUDCOLOR_H_
#define CLOUDCOLOR_H_

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <math.h>
#include <Arduino.h>
#include "../Property.h"

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class Color {
  public:
    float hue, sat, bri;
    Color(float h, float s, float b): hue(h), sat(s), bri(b) {
      setColorHSB(h, s, b);
    }

    bool setColorHSB(float h, float s, float b) {
      if (h < 0 || h > 360 || s < 0 || s > 100 || b < 0 || b > 100) {
        hue = 0;
        sat = 0;
        bri = 0;
        return false;
      }

      hue = h;
      sat = s;
      bri = b;
      return true;
    }

    bool setColorRGB(uint8_t R, uint8_t G, uint8_t B) {
      float temp[3];
      float max, min, delta;
      uint8_t imax;
      temp[0] = (float)R / 255;
      temp[1] = (float)G / 255;
      temp[2] = (float)B / 255;
      max = temp[0];
      imax = 0;
      min = temp[0];

      for (uint8_t j = 0; j < 3; j++) {

        if (temp[j] >= max) {
          max = temp[j];
          imax = j;
        }
        if (temp[j] <= min) {
          min = temp[j];
        }
      }

      delta = max - min;
      if (delta == 0) {
        hue = 0;
      } else if (imax == 0) {

        hue = 60 * fmod((temp[1] - temp[2]) / delta, 6);
      } else if (imax == 1) {
        hue = 60 * (((temp[2] - temp[0]) / delta) + 2);
      } else if (imax == 2) {
        hue = 60 * (((temp[0] - temp[1]) / delta) + 4);
      }

      if (max == 0) {
        sat = 0;
      } else {
        sat = (delta / max) * 100;
      }

      bri = max * 100;
      return true;
    }

    void getRGB(uint8_t& R, uint8_t& G, uint8_t& B) {
      float fC = (bri / 100) * (sat / 100);
      float fHPrime = fmod(hue / 60.0, 6);
      float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
      float fM = (bri / 100) - fC;
      float fR, fG, fB;

      if (0 <= fHPrime && fHPrime < 1) {
        fR = fC;
        fG = fX;
        fB = 0;
      } else if (1 <= fHPrime && fHPrime < 2) {
        fR = fX;
        fG = fC;
        fB = 0;
      } else if (2 <= fHPrime && fHPrime < 3) {
        fR = 0;
        fG = fC;
        fB = fX;
      } else if (3 <= fHPrime && fHPrime < 4) {
        fR = 0;
        fG = fX;
        fB = fC;
      } else if (4 <= fHPrime && fHPrime < 5) {
        fR = fX;
        fG = 0;
        fB = fC;
      } else if (5 <= fHPrime && fHPrime < 6) {
        fR = fC;
        fG = 0;
        fB = fX;
      } else {
        fR = 0;
        fG = 0;
        fB = 0;
      }
      R = lrint((fR + fM) * 255);
      G = lrint((fG + fM) * 255);
      B = lrint((fB + fM) * 255);
    }

    Color& operator=(Color & aColor) {
      hue = aColor.hue;
      sat = aColor.sat;
      bri = aColor.bri;
      return *this;
    }

    bool operator==(Color & aColor) {
      return hue == aColor.hue && sat == aColor.sat && bri == aColor.bri;
    }

    bool operator!=(Color & aColor) {
      return !(operator==(aColor));
    }

};

class CloudColor : public Property {
  private:
    Color _value,
          _cloud_value;
  public:
    CloudColor() : _value(0, 0, 0), _cloud_value(0, 0, 0) {}
    CloudColor(float hue, float saturation, float brightness) : _value(hue, saturation, brightness), _cloud_value(hue, saturation, brightness) {}

    virtual bool isDifferentFromCloud() {

      return _value != _cloud_value;
    }

    CloudColor& operator=(Color aColor) {
      _value.hue = aColor.hue;
      _value.sat = aColor.sat;
      _value.bri = aColor.bri;
      updateLocalTimestamp();
      return *this;
    }

    Color getCloudValue() {
      return _cloud_value;
    }

    Color getValue() {
      return _value;
    }

    virtual void fromCloudToLocal() {
      _value = _cloud_value;
    }
    virtual void fromLocalToCloud() {
      _cloud_value = _value;
    }
    virtual CborError appendAttributesToCloud(CborEncoder *encoder) {
      CHECK_CBOR_MULTI(appendAttribute(_value.hue, "hue", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.sat, "sat", encoder));
      CHECK_CBOR_MULTI(appendAttribute(_value.bri, "bri", encoder));
      return CborNoError;
    }
    virtual void setAttributesFromCloud() {
      setAttribute(_cloud_value.hue, "hue");
      setAttribute(_cloud_value.sat, "sat");
      setAttribute(_cloud_value.bri, "bri");
    }
};

#endif /* CLOUDCOLOR_H_ */