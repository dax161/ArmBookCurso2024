#include "mbed.h"
#include ""
DigitalIn Button(D2);

AnalogIn sensor1(A0);
AnalogIn sensor2(A1);

DigitalOut alarmBuzzer(D1);
DigitalOut alarmLed(D0);

bool alarmSate = OFF;

void inputsInit()
{
    alarmBuzzer.mode(PullDown);

}

int main()
{
}
