/*
 * BasicsLibrary.h - A library for beginers with no or basic arduino knowledge.
 * Created by Tomas Roj, March third 2019.
 * Open source, free to use.
 */

#include "Arduino.h"
#include "BasicsLibrary.h"

BasicsLibrary::BasicsLibrary(int pin){
  _pin = pin;
}

void BasicsLibrary::btnSetup(int buttonPin){
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(_pin, HIGH);
}

void BasicsLibrary::blinking(){
  digitalWrite(_pin, HIGH);
  delay(1000);
  digitalWrite(_pin, LOW);
  delay(1000);
}

void BasicsLibrary::serial(String text){
  Serial.begin(9600);
  Serial.println(text);
  delay(1500);
}

void BasicsLibrary::btn(int buttonPin){

  if (digitalRead(buttonPin) == LOW){
    digitalWrite(_pin, LOW);
    delay(5000);
    digitalWrite(_pin, HIGH);
  }
  else if(digitalRead(buttonPin) == HIGH){
    digitalWrite(_pin, HIGH);
  }

}

void BasicsLibrary::rgbLed(int red, int green, int blue){

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  digitalWrite(red, HIGH);
  delay(1000);
  digitalWrite(red, LOW);
  digitalWrite(green, HIGH);
  delay(1000);
  digitalWrite(green, LOW);
  digitalWrite(blue, HIGH);
  delay(1000);
  digitalWrite(blue, LOW);
  delay(100);

}

void BasicsLibrary::servoTurning(int dataPin){
  #include <Servo.h>     
  Servo myservo;         
  int pos = 0;          

  myservo.attach(dataPin);   

  for(pos = 0; pos <= 180; pos += 1) 
  {
    myservo.write(pos);  
    delay(15);           
  } 
  for(pos = 180; pos >= 0; pos -= 1)
  {
    myservo.write(pos);  
    delay(15);           
  }

}
