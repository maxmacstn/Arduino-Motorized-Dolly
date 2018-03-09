# Arduino-Motorized-Dolly
Convert generic camera dolly rail into controllable motorized dolly.

![Arduino Dolly](https://raw.githubusercontent.com/maxmacstn/Arduino-Motorized-Dolly/master/img/photo6129581745381681141.jpg)

## Features
- 5v powerbank input (Portable!)
- Adjustable speed.
- Video mode (Continuous movement)
- TimeLapse mode with camera shutter trigger.

## Main Components
1. Andoer 80cm / 32" 4 Bearings Camera Slider
2. Arduino Mega 2560
3. Gear motor 12v 50RPM
4. Arduino Modules
    1. L298N H-Bridge motor driver
    2. LCD Keypad shield
    3. DC-to-DC Step Up XL6009
    4. Limit switch ×2
5. Belt components
    1. timing GT2 belt width 6mm
    2. Timing Pulley 20 ฟัน 2GT 
    3. Pulley Wheel 20 ฟัน 2GT
    4. Timing Belt Fixing Clamp 9*40mm
6. Shutter trigger components
    1. NPN Transistor BC547
    2. Resistor 1K Ω
    3. 2.5mm female headphone jack

## Schematic
![Schematic](https://raw.githubusercontent.com/maxmacstn/Arduino-Motorized-Dolly/master/img/DollyDuino_schematic_bb.jpg)
## Dependencies
- LiquidCrystal.h

## Link
- [Youtube Link](https://youtu.be/vfO5Gkq3VD4)
- [Blog(Thai)](https://maxmacstn.wordpress.com/2018/03/09/dollyduino/)
