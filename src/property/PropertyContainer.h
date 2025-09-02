/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2020 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_PROPERTY_CONTAINER_H_
#define ARDUINO_PROPERTY_CONTAINER_H_

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include "Property.h"

#undef max
#undef min
#include <list>

#include "types/CloudBool.h"
#include "types/CloudFloat.h"
#include "types/CloudInt.h"
#include "types/CloudUnsignedInt.h"
#include "types/CloudString.h"
#include "types/CloudLocation.h"
#include "types/CloudSchedule.h"
#include "types/CloudColor.h"
#include "types/CloudWrapperBase.h"

#include "types/automation/CloudColoredLight.h"
#include "types/automation/CloudContactSensor.h"
#include "types/automation/CloudDimmedLight.h"
#include "types/automation/CloudLight.h"
#include "types/automation/CloudMotionSensor.h"
#include "types/automation/CloudSmartPlug.h"
#include "types/automation/CloudSwitch.h"
#include "types/automation/CloudTemperatureSensor.h"
#include "types/automation/CloudTelevision.h"

/******************************************************************************
  DECLARATION OF getTime
 ******************************************************************************/

extern "C" unsigned long getTime();

/******************************************************************************
  TYPEDEF
 ******************************************************************************/

typedef std::list<Property *> PropertyContainer;

typedef CloudFloat CloudEnergy;
typedef CloudFloat CloudForce;
typedef CloudFloat CloudTemperature;
typedef CloudFloat CloudPower;
typedef CloudFloat CloudElectricCurrent;
typedef CloudFloat CloudElectricPotential;
typedef CloudFloat CloudElectricResistance;
typedef CloudFloat CloudCapacitance;
typedef CloudUnsignedInt CloudTime;
typedef CloudFloat CloudFrequency;
typedef CloudFloat CloudDataRate;
typedef CloudFloat CloudHeartRate;
typedef CloudInt   CloudCounter;
typedef CloudFloat CloudAcceleration;
typedef CloudFloat CloudArea;
typedef CloudFloat CloudLength;
typedef CloudFloat CloudVelocity;
typedef CloudFloat CloudMass;
typedef CloudFloat CloudVolume;
typedef CloudFloat CloudFlowRate;
typedef CloudFloat CloudAngle;
typedef CloudFloat CloudIlluminance;
typedef CloudFloat CloudLuminousFlux;
typedef CloudFloat CloudLuminance;
typedef CloudFloat CloudLuminousIntensity;
typedef CloudFloat CloudLogarithmicQuantity;
typedef CloudFloat CloudPressure;
typedef CloudInt   CloudInformationContent;
typedef CloudFloat CloudPercentage;
typedef CloudFloat CloudRelativeHumidity;

/******************************************************************************
  FUNCTION DECLARATION
 ******************************************************************************/

Property & addPropertyToContainer(PropertyContainer & prop_cont,
                                  Property & property,
                                  String const & name,
                                  Permission const permission,
                                  int propertyIdentifier = -1,
                                  GetTimeCallbackFunc func = getTime);


Property * getProperty(PropertyContainer & prop_cont, String const & name);
Property * getProperty(PropertyContainer & prop_cont, int const identifier);


void updateTimestampOnLocallyChangedProperties(PropertyContainer & prop_cont);
void requestUpdateForAllProperties(PropertyContainer & prop_cont);
void updateProperty(PropertyContainer & prop_cont, String propertyName, unsigned long cloudChangeEventTime, bool const is_sync_message, std::list<CborMapData> * map_data_list);
String getPropertyNameByIdentifier(PropertyContainer & prop_cont, int propertyIdentifier);

#endif /* ARDUINO_PROPERTY_CONTAINER_H_ */
