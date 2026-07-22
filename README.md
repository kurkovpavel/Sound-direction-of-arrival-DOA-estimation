This is a fork of https://github.com/respeaker/seeed-voicecard
Folder seeed-voicecard_driver_for_kernel6.1.21 has updated source code for building and installing drivers of sound cards for Linux raspberry 6.1.21-v8+. Follows README instructions inside for compilling

Folder doa_estimation is an example of DOA estimation for that sound card.
Install:

make clean
make

Running:
./doa_estimation
========================================
  DOA System with LED Shift
========================================

LED init...
LED ready

Starting...

Controls:
  Up/Down: Shift LED direction by 5°
  Left/Right: Shift LED direction by 1° (fine)
  S: Toggle shift ON/OFF
  R: Reset shift to 0
  Q: Quit
========================================
Failed to set thread affinity: No such device or address
DOA running
---------------------------------------------
[66] DOA: 136.0°

Run python3 checkgpio.py if LEDs is not light up
