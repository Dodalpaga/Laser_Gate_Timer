# Laser_Gate_Timer
## This repo aims to create two laser gate timer cells.

What you will need for this project :
- 2 STM32-F411-RE Microcontrollers (.bin programs)
- 1 IDB0XA1-5 BLE Shield
- 1 433MHz Emitter
- 1 433MHz Receiver
- 1 KY008 Laser Module (or any laser with analog output)
- 1 SD 5600 Photodiode Module  (or any photodiode with analog/digital output)
- 2 mirrors
- 1 .apk mobile app for controlling the cells and monitoring timers 

How its done :
- 1 Starting Cell : STM32-F411 + 433 MHz Emitter + Laser Photodiode + A mirror to create a laser gate
- 1 Finish Cell : STM32-F411 + 433 MHz Receiver + BLE + Laser Photodiode + A mirror to create a laser gate
- 1 mobile app connected to Finish cell via BLE

Note : You can find more projects of my class on this link : https://os.mbed.com/teams/ENSMM/
