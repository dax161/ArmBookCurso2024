#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn Button(D2);
AnalogIn sensorDeteccion1(A0);
AnalogIn sensorDeteccion2(A1);

DigitalOut alarmaLed(D0);
DigitalOut alarmaBuzzer(D1);

UnbufferedSerial uartUsb(USBTX, USBRX, 9600);

int aforo_actual = 0;
int aforo_maximo = 40;
bool estado_alarma = OFF;

void inputsInit()
{
    Button.mode(PullDown);
}

void outputsInit()
{
    estado_alarma = OFF;
    alarmaLed = OFF;
    alarmaBuzzer = OFF;
}

void imprimirAforo()
{
    char str[30];
    sprintf ( str, "Aforo: %d/%d\r\n", aforo_actual,aforo_maximo );
    uartUsb.write( str, strlen(str) );
}

int procesarSensores()
{
    int cont1 = 0;
    int cont2 = 0;
    int distanciaDetectada1;
    int distanciaDetectada2;
    bool sensores_activados = false;

    while (!sensores_activados)
    {
        distanciaDetectada1 = ((sensorDeteccion1.read() * 5) * 3072);
        distanciaDetectada2 = ((sensorDeteccion2.read() * 5) * 3072);

        if (distanciaDetectada1 < 1000) { 
            if (cont2 == 0) {
                cont1 = 2;
            } else if (cont2 == 1) {
                cont1 = 1;
            }
        }

        if (sensor2Reading > 2.0) { // Ajusta el umbral según tu sensor
            if (cont1 == 0) {
                cont2 = 2;
            } else if (cont1 == 1) {
                cont2 = 1;
            }
        }

        if ((cont1 + cont2) > 3) {
            if (cont1 == 2) {
                aforo_actual++;
            } else if (cont2 == 2) {
                aforo_actual--;
            }
            cont1 = 0;
            cont2 = 0;
            sensores_activados = true;
        }
    }
}

bool activacion_desactivacion_Alarma(int aforo_actual, int aforo_maximo)
{
    while (aforo_actual > aforo_maximo)
    {
        alarmLed = ON;
        alarmBuzzer = ON;
        if (Button.read() == 1) {
            alarmaLed = OFF;
            alarmaBuzzer = OFF;
            return false; 
        }
    }

    return true; 
}

int main()
{   
    inputsInit();
    outputsInit();
}
