/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef ARDUINO_OPTA
#include "OptaFactoryTest.h"
#include "utility/ResetInput.h"

#define VID_FINDER            0x35D1
#define VID_ARDUINO           0x2341

#define PID_BASIC             0x0064
#define PID_PLUS              0x0164
#define PID_ADVANCED          0x0264

#define LED1_SYS              PI_0   // 154u
#define LED2_SYS              PI_1   // 155u
#define LED3_SYS              PI_3   // 157u
#define LED4_SYS              PH_15  // 153u

#define RL1                   D0
#define RL2                   D1
#define RL3                   D2
#define RL4                   D3

#define N_ANALOG_INPUTS       8
#define N_LED                 7

#define ANALOG_THS            2.0

#define MY_RS485_TX_PIN         PB_10
#define MY_RS485_RX_PIN         PB_11
#define MY_RS485_DE_PIN         PB_14
#define MY_RS485_RE_PIN         PB_13

#define N_PULSE                 54

/* Constants */
const uint8_t n_input[N_ANALOG_INPUTS] = {A0, A1, A2, A3, A4, A5, A6, A7};
const uint8_t n_led[N_LED] = {LEDR, LEDG, LEDB, 154u, 155u, 157u, 153u};
float v_input[N_ANALOG_INPUTS];

OptaBoardInfo* boardInfo();

void OptaFactoryTestClass::begin() {
  pinMode(LED1_SYS, OUTPUT);
  pinMode(LED2_SYS, OUTPUT);
  pinMode(LED3_SYS, OUTPUT);
  pinMode(LED4_SYS, OUTPUT);

  pinMode(RL1, OUTPUT);
  pinMode(RL2, OUTPUT);
  pinMode(RL3, OUTPUT);
  pinMode(RL4, OUTPUT);

  /* Set ADC resolution to 12 bits */
  analogReadResolution(12);
  do {
    _info = boardInfo();
  } while (_info == nullptr);
  if(_info->_board_functionalities.rs485 == 1) {
    ResetInput::getInstance().setPinChangedCallback(buttonCallbackRS485);
  } else {
    ResetInput::getInstance().setPinChangedCallback(endCallback);
  }

}
void OptaFactoryTestClass::optaIDTest() {
  if(_info->magic == 0xB5) {
    if(_info->vid == VID_FINDER) {
      if(_info->pid == PID_BASIC) {
        digitalWrite(RL4, HIGH);
        digitalWrite(LED4_SYS, HIGH);
      }
      else {
        if(_info->pid == PID_PLUS) {
          digitalWrite(RL3, HIGH);
          digitalWrite(LED3_SYS, HIGH);
        }
        else {
          if(_info->pid == PID_ADVANCED) {
            digitalWrite(RL4, HIGH);
            digitalWrite(LED4_SYS, HIGH);
            digitalWrite(RL3, HIGH);
            digitalWrite(LED3_SYS, HIGH);
          }
        }
      }
    }
    else {
      if(_info->vid == VID_ARDUINO) {
        if(_info->pid == PID_BASIC) {
          digitalWrite(RL2, HIGH);
          digitalWrite(LED2_SYS, HIGH);
        }
        else {
          if(_info->pid == PID_PLUS) {
            digitalWrite(RL2, HIGH);
            digitalWrite(LED2_SYS, HIGH);
            digitalWrite(RL4, HIGH);
            digitalWrite(LED4_SYS, HIGH);
          }
          else {
            if(_info->pid == PID_ADVANCED) {
              digitalWrite(RL2, HIGH);
              digitalWrite(LED2_SYS, HIGH);
              digitalWrite(RL3, HIGH);
              digitalWrite(LED3_SYS, HIGH);
            }
          }
        }
      }
    }
    delay(1000);
    digitalWrite(RL1, LOW);
    digitalWrite(LED1_SYS, LOW);
    digitalWrite(RL2, LOW);
    digitalWrite(LED2_SYS, LOW);
    digitalWrite(RL3, LOW);
    digitalWrite(LED3_SYS, LOW);
    digitalWrite(RL4, LOW);
    digitalWrite(LED4_SYS, LOW);
  }

  /* Turn ON all LED */
  for(uint8_t i = 0; i < N_LED; i++) {
    digitalWrite(n_led[i], HIGH);
    delay(50);
  }
  /* Turn OFF all LED */
  for(uint8_t i = 0; i < N_LED; i++) {
    digitalWrite(n_led[i], LOW);
    delay(50);
  }


  printInfo();

  if(_info->_board_functionalities.rs485 == 1) {
    pinMode(MY_RS485_TX_PIN, OUTPUT);
    digitalWrite(MY_RS485_TX_PIN, LOW);

    pinMode(MY_RS485_DE_PIN, OUTPUT);
    digitalWrite(MY_RS485_DE_PIN, LOW);

    pinMode(MY_RS485_RE_PIN, OUTPUT);
    digitalWrite(MY_RS485_RE_PIN, LOW);

    pinMode(MY_RS485_RX_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(MY_RS485_RX_PIN), rs485Rcv, FALLING);
    _nextBoardInfoPrint = millis();
    _nextRS485Run = millis() + 2000;
  }

}

bool OptaFactoryTestClass::poll() {
  if(_ms10 < millis()) {
    _ms10 = millis() + 10;
    ledManage();
  }
  if(_ms100 < millis()) {
    _ms100 = millis() + 100;

    inputManage();

  }

  if (_showRS485Result) {
    showRS485SuccessResult();
  }

  if (_nextBoardInfoPrint < millis()) {
    _nextBoardInfoPrint = millis() + 3000;
    Serial.print(millis());
    Serial.println("ms");
    printModel();
  }

  if(_nextRS485Run < millis() && _info->_board_functionalities.rs485 == 1 && _rs485_test_done == false) {
    _nextRS485Run = millis() + 1000;
    rs485Manage();
  }
  return _test_running;
}

void OptaFactoryTestClass::ledManage(void)
{
  /* Run every 10ms */
  if(_all_on) {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
  }
}

void OptaFactoryTestClass::inputManage(void)
{
  /* Read all analog input values */
  for(uint8_t i = 0; i < N_ANALOG_INPUTS; i++) {
    v_input[i] = analogRead(n_input[i]) * (3.3 / 4095.0);
  }
  if(v_input[0] >= ANALOG_THS) {
    _test_running = true;
    digitalWrite(RL1, HIGH);
    digitalWrite(LED1_SYS, HIGH);
  }
  if(v_input[1] >= ANALOG_THS) {
    digitalWrite(RL2, HIGH);
    digitalWrite(LED2_SYS, HIGH);
  }
  if(v_input[2] >= ANALOG_THS) {
    digitalWrite(RL3, HIGH);
    digitalWrite(LED3_SYS, HIGH);
  }
  if(v_input[3] >= ANALOG_THS) {
    digitalWrite(RL4, HIGH);
    digitalWrite(LED4_SYS, HIGH);
  }
  if(v_input[4] >= ANALOG_THS) {
    digitalWrite(RL1, HIGH);
    digitalWrite(LED1_SYS, HIGH);
    digitalWrite(RL2, HIGH);
    digitalWrite(LED2_SYS, HIGH);
  }
  if(v_input[5] >= ANALOG_THS) {
    digitalWrite(RL3, HIGH);
    digitalWrite(LED3_SYS, HIGH);
    digitalWrite(RL4, HIGH);
    digitalWrite(LED4_SYS, HIGH);
  }
  if(v_input[6] >= ANALOG_THS) {
    digitalWrite(RL1, HIGH);
    digitalWrite(LED1_SYS, HIGH);
    digitalWrite(RL3, HIGH);
    digitalWrite(LED3_SYS, HIGH);
  }
  if(v_input[7] >= ANALOG_THS) {
    digitalWrite(RL2, HIGH);
    digitalWrite(LED2_SYS, HIGH);
    digitalWrite(RL4, HIGH);
    digitalWrite(LED4_SYS, HIGH);
  }

  if((v_input[0] < ANALOG_THS) && (v_input[4] < ANALOG_THS) && (v_input[6] < ANALOG_THS)) {
    digitalWrite(RL1, LOW);
    digitalWrite(LED1_SYS, LOW);
  }
  if((v_input[1] < ANALOG_THS) && (v_input[4] < ANALOG_THS) && (v_input[7] < ANALOG_THS)) {
    digitalWrite(RL2, LOW);
    digitalWrite(LED2_SYS, LOW);
  }
  if((v_input[2] < ANALOG_THS) && (v_input[5] < ANALOG_THS) && (v_input[6] < ANALOG_THS)) {
    digitalWrite(RL3, LOW);
    digitalWrite(LED3_SYS, LOW);
  }
  if((v_input[3] < ANALOG_THS) && (v_input[5] < ANALOG_THS) && (v_input[7] < ANALOG_THS)) {
    digitalWrite(RL4, LOW);
    digitalWrite(LED4_SYS, LOW);
  }
  if((v_input[0] >= ANALOG_THS) && (v_input[1] >= ANALOG_THS) && (v_input[2] >= ANALOG_THS) && (v_input[3] >= ANALOG_THS) && (v_input[4] >= ANALOG_THS) && (v_input[5] >= ANALOG_THS) && (v_input[6] >= ANALOG_THS) && (v_input[7] >= ANALOG_THS)) {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
    _all_on = true;
  }
  else if( _all_on == true ) {
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
    _all_on = false;
  }
}

void OptaFactoryTestClass::rs485Manage() {
  uint32_t t_rs485_pulse = 0;
  digitalWrite(MY_RS485_RE_PIN, HIGH);
  digitalWrite(MY_RS485_DE_PIN, HIGH);
  delay(10);

  for(uint32_t i = 0; i < N_PULSE; i++) {
    digitalWrite(MY_RS485_TX_PIN, HIGH);
    delay(1);
    digitalWrite(MY_RS485_TX_PIN, LOW);
    delay(1);
  }
  delay(10);
  digitalWrite(MY_RS485_DE_PIN, LOW);
  digitalWrite(MY_RS485_RE_PIN, LOW);

  /* Search start of incoming transmission */
  _rs485_pulse = 0;
  while((_rs485_pulse == 0) && (t_rs485_pulse < 200)) {
    t_rs485_pulse++;
    delay(1);
  }

  _rs485_pulse = 0;

  if(t_rs485_pulse < 200) {
    /* Receive data */
    t_rs485_pulse = 0;
    uint32_t rs485_pulse_old = 0;
    for(t_rs485_pulse = 0; t_rs485_pulse < 20; t_rs485_pulse++) {
      if(rs485_pulse_old != _rs485_pulse) {
        rs485_pulse_old = _rs485_pulse;
        t_rs485_pulse = 0;
      }
      delay(10);
    }

    /* End of receiving */
    if(_rs485_pulse > 0) {
      if((_rs485_pulse == N_PULSE) || (_rs485_pulse == N_PULSE + 1)) {
        Serial.println("RS485 check OK");
        _rs485_test_done = true;
        _rs485_ok = true;
      }
    }
  }

}


void OptaFactoryTestClass::printInfo() {
  Serial.print("\n");
  Serial.print("**********************************\n");
  Serial.print(">>> OPTA - 2023-07-13 15:38:21 <<<\n");
  Serial.print("**********************************\n");
  Serial.print("\n");

  if(_info->magic == 0xB5) {
    printModel();

    Serial.println("VID: 0x" + String(_info->vid, HEX));
    Serial.println("PID: 0x" + String(_info->pid, HEX));

    char mac_address_char[18];
    uint8_t idx_mac_address_char = 0;
    uint8_t a;
    for(uint8_t i = 0; i < 6; i++) {
      a = _info->mac_address[i] >> 4;
      for(uint8_t b = 0; b < 2; b++) {
        if(a <= 9) {
          mac_address_char[idx_mac_address_char] = a + 0x30;
        }
        else {
          mac_address_char[idx_mac_address_char] = a + 0x37;
        }
        idx_mac_address_char++;
        a = _info->mac_address[i] & 0x0F;
      }
      if(i < 5) {
        mac_address_char[idx_mac_address_char++] = ':';
      }
    }
    mac_address_char[idx_mac_address_char] = 0;
    Serial.println("MAC: " + String(mac_address_char));

    Serial.println("Has Ethernet: " + String(_info->_board_functionalities.ethernet == 1 ? "Yes" : "No"));
    Serial.println("Has WiFi module: " + String(_info->_board_functionalities.wifi == 1 ? "Yes" : "No"));
    Serial.println("Has RS485: " + String(_info->_board_functionalities.rs485 == 1 ? "Yes" : "No"));
  }


}

void OptaFactoryTestClass::printModel(void)
{
  switch(_info->vid) {
    case VID_FINDER:
    {
      Serial.print(">>> Finder OPTA ");
      switch(_info->pid) {
        case PID_BASIC:
        {
          Serial.print("Basic");
        } break;

        case PID_PLUS:
        {
          Serial.print("Plus");
        } break;

        case PID_ADVANCED:
        {
          Serial.print("Advanced");
        } break;

        default:
        {
          Serial.print("Unknown");
        } break;
      }
    } break;

    case VID_ARDUINO:
    {
      Serial.print(">>> Arduino OPTA ");
      switch(_info->pid) {
        case PID_BASIC:
        {
          Serial.print("Lite - AFX00003");
        } break;

        case PID_PLUS:
        {
          Serial.print("RS485 - AFX00001");
        } break;

        case PID_ADVANCED:
        {
          Serial.print("WiFi - AFX00002");
        } break;

        default:
        {
          Serial.print("Unknown");
        } break;
      }
    } break;

    default:
    {
      Serial.print("Unknown ");
    } break;
  }
  Serial.println(" <<<\n");
}

void OptaFactoryTestClass::endCallback() {
  if(digitalRead(BTN_USER) == HIGH) {
    _test_running = false;
  }
}

void OptaFactoryTestClass::buttonCallbackRS485()
{

  if(digitalRead(BTN_USER) == LOW && _rs485_ok == true) {
    _showRS485Result = true;
  }
}

void OptaFactoryTestClass::showRS485SuccessResult() {
  digitalWrite(RL1, HIGH);
  digitalWrite(LED1_SYS, HIGH);
  digitalWrite(RL4, HIGH);
  digitalWrite(LED4_SYS, HIGH);
  delay(1000);
  digitalWrite(RL1, LOW);
  digitalWrite(LED1_SYS, LOW);
  digitalWrite(RL4, LOW);
  digitalWrite(LED4_SYS, LOW);
  _test_running = false;
  _showRS485Result = false;

}

void OptaFactoryTestClass::rs485Rcv() {
  _rs485_pulse++;
}

OptaFactoryTestClass OptaFactoryTest;

#endif
