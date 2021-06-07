#include "mbed.h"
///////////Init Device/////////////////
Serial device(D8, D2);
Serial pc(USBTX, USBRX);
/////////////////////////////////////

///////////Init STM32 pins///////////
DigitalOut myLed(LED1);
DigitalOut compteur(A3);
DigitalOut laser(A5);
InterruptIn photodiode(A4);
InterruptIn my_button(USER_BUTTON);  //User1
/////////////////////////////////////


/////////COMMUNICATION///////////////
void device_nodetect()
{
    compteur = 1;
    wait(0.001);
    compteur = 0;
    for( int h = 0; h < 2; h++ ) {
        device.putc(0xBB);
        device.putc(0xBB);
        device.putc(0xBB);
        device.putc(0xBC);
        device.putc(0xBC);
        device.putc(0xBC);
        device.putc(0xBD);
        device.putc(0xBD);
        device.putc(0xBD);
    }
    
}
void device_detect()
{
    compteur = 1;
    wait(0.001);
    compteur = 0;
    for( int h = 0; h < 10; h++ ) {
        device.putc(0xCC);
    }
}
/////////////////////////////////////


int main(int, char**)
{
    device.baud(2400);
    my_button.fall(&device_nodetect);
    photodiode.rise(&device_detect);
    photodiode.fall(&device_nodetect);
    laser = 1;

    while (1) {
        //while RF device is receiving data
        //RF Transmit Code
        //Send 10101010 pattern when idle to keep receiver in sync and locked to transmitter
        //When receiver loses the sync lock (Around 30MS with no data change seen) it starts sending out noise
        device.putc(0xAA);
        for( int h = 0; h < 10; h++ ) {
            
            device.putc('x');
        }
        wait(1);
    }
}