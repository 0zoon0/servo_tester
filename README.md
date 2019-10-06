# servo_tester

Using an ATtiny13A micro-controller to run a servo. Code is running the servo from 0 to 100 continuously. The button on the board can switch through various speeds.

The chip is running at 9.8Mhz and every 96 counts an interrupt is fired (i.e. 100kHz). The interrupt is counting up to 2000 to detect a new frame (i.e. 20ms).

The two LEDs on the board are showing servo direction so that there is a visual indication that CPU is doing something. 