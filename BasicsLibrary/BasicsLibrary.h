/*
 * BasicsLibrary.h - A library for beginers with no or basic arduino knowledge.
 * Created by Tomas Roj, March third 2019.
 * Open source, free to use.
 */

#ifndef BasicsLibrary_h
#define BasicsLibrary_h

#include "Arduino.h"

class BasicsLibrary
{
  public:
    BasicsLibrary(int pin);
    void blinking();
    void serial(String text);
    void btn(int button);
    void btnSetup(int button);
    void rgbLed(int red, int green, int blue);
    void servoTurning(int dataPin);
  private:
    int _pin;
};

#endif
