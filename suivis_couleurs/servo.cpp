#include "mbed.h"
#include "LittleFileSystem.h"

#define STEP 2

DigitalOut led1(LED1);
DigitalOut led2(LED2);

PwmOut servoCam(P2_4);
PwmOut servoBase(P2_3);

static BufferedSerial pc(USBTX, USBRX); // tx, rx

int main()
{
    int cx;

    float offsetB=1500;
    float offsetC=1500;

    char msg[] = "\nStart new input :\n";
    char *c = new char[1];
    pc.write(msg, sizeof(msg));

    while (1) {

        servoCam.pulsewidth_us(offsetC);
        servoBase.pulsewidth_us(offsetB);

        pc.read(c, sizeof(c));
        pc.write(c, sizeof(c));
	 if (*c == 'r') {
            offsetB=1500;
            offsetC=1500;
            led1=0;
            led2=0;
        }
	
        if (*c == 'w') {
            offsetC-=STEP;
            led1=1;
            led2=0;
        }
        if (*c == 's') {
            offsetC+=STEP;
            led1=0;
            led2=1;
        }
        if (*c == 'd') {
            offsetB+=STEP;
            led1=0;
            led2=1;
        }
        if (*c == 'a') {
            offsetB-=STEP;
            led1=0;
            led2=1;
        }
    }
}
