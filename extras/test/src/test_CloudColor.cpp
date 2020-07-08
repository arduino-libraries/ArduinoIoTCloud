/*
   Copyright (c) 2019 Arduino.  All rights reserved.
*/

/**************************************************************************************
   INCLUDE
 **************************************************************************************/

#include <catch.hpp>

#include <util/CBORTestUtil.h>

#include <property/types/CloudColor.h>

/**************************************************************************************
  TEST CODE
 **************************************************************************************/

SCENARIO("Arduino Cloud Properties ", "[ArduinoCloudThing::CloudColor]")
{
  WHEN("Set invalid color HSB")
  {
    CloudColor color_test = CloudColor(0.0, 0.0, 0.0);

    Color value_color_test = color_test.getValue();
    REQUIRE(value_color_test.setColorHSB(500.0, 20.0, 30.0) == false);
  }

  WHEN("Set and Get different RGB colors")
  {
    uint8_t r, g, b;

    CloudColor color_test = CloudColor(0.0, 0.0, 0.0);

    Color value_color_test = color_test.getValue();

    value_color_test.setColorRGB(128, 64, 64);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 128);
    REQUIRE(g == 64);
    REQUIRE(b == 64);

    value_color_test.setColorRGB(126, 128, 64);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 126);
    REQUIRE(g == 128);
    REQUIRE(b == 64);

    value_color_test.setColorRGB(64, 128, 64);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 64);
    REQUIRE(g == 128);
    REQUIRE(b == 64);

    value_color_test.setColorRGB(64, 64, 128);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 64);
    REQUIRE(g == 64);
    REQUIRE(b == 128);

    value_color_test.setColorRGB(255, 0, 255);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 255);
    REQUIRE(g == 0);
    REQUIRE(b == 255);

    value_color_test.setColorRGB(0, 0, 0);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 0);
    REQUIRE(g == 0);
    REQUIRE(b == 0);

    value_color_test.setColorRGB(50, 100, 20);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 50);
    REQUIRE(g == 100);
    REQUIRE(b == 20);

    value_color_test.setColorRGB(20, 50, 70);
    value_color_test.getRGB(r, g, b);

    REQUIRE(r == 20);
    REQUIRE(g == 50);
    REQUIRE(b == 70);
  }

  WHEN("Set HSB colors and get RGB")
  {
    bool verify;
    uint8_t r, g, b;

    CloudColor color_test = CloudColor(0.0, 0.0, 0.0);

    Color value_color_test = color_test.getValue();

    value_color_test.setColorHSB(240, 50, 50);
    value_color_test.getRGB(r, g, b);
    verify = r == 64 && g == 64 && b == 128;

    REQUIRE(verify);

    value_color_test.setColorHSB(120, 50, 50);
    value_color_test.getRGB(r, g, b);
    verify = r == 64 && g == 128 && b == 64;

    REQUIRE(verify);
  }
}