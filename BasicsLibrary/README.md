## Welcome to BasicsLibrary docs.
An easy to-use library for beginners with examples to start their adventure with Arduino.
The whole tutorial is also on - https://tomasroj.github.io/BasicsLibrary/.

### What is it & why you should use it.

BasicsLibrary is an open-source library for Arduino beginners. With easy to-use experience I made it. If you or your friend is having trouble with the basics of Arduino, my library will probably help. All you must know is how to use a breadboard, connect wires and electronic components to it. And that's all. Don't worry about source code again! With simple methods included, you can just call one and focus on hardware.

### How to set up:

1. Download and install BasicsLibrary from Arduino library manager (CTRL + SHIFT + I).
2. Use the following code in your Arduino sketch:

```c++
#include <BasicsLibrary.h> //imports the library

BasicsLibrary instance(4);
//Creates the BasicsLibrary instance. There must be pin number even
//if you don't must to.
```
3. And that's it! Then just use your breadboard and one of the following methods with the instance name.
ex. `instanceName.blinking()`.

### All methods - usage and requirements.

```c++
btnSetup(button pin);
//Does the required setup of button.

btn(button pin);
//Defines code of button. Must be used with the btnSetup() method.

blinking();
//Led blinking. Uses pin from instance creation.

serial(String text);
//Starts and prints text to serial (9600 baud). Must be used with the string with text.

rgbLed(int red, int green, int blue);
//RGB led blinking with all of its colors. You must define pin of all three diode pins.

servoTurning(int dataPin);
//Turns servo from 0 to 180 degrees. Must be included with OUT pin of servo.
```

### Support or Contact

Still having trouble with BasicsLibrary? Write me on email - rojtomas@email.cz and I can help you with that! Or visit me on my personal website - www.tomasroj.herokuapp.com. Thanks for support! PS: Don't worry to contribute!
