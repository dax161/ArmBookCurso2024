#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn Button(D2);
AnalogIn sensorDeteccion1(A0);
AnalogIn sensorDeteccion2(A1);

DigitalOut alarmaLed(D0);
DigitalOut alarmaBuzzer(D1);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int aforo_actual = 0;
int aforo_maximo = 40;
bool estado_alarma;
bool pausa_alarma;

void inputsInit()
{
    Button.mode(PullDown);
}

void outputsInit()
{
    pausa_alarma = OFF;
    alarmaLed = OFF;
    alarmaBuzzer = OFF;
}

void lectura_sensores(int distanciaDetectada1, int distanciaDetectada2)
{
    distanciaDetectada1 = ((sensorDeteccion1.read() * 5) * 3072);
    distanciaDetectada2 = ((sensorDeteccion2.read() * 5) * 3072);
}
void deteccion_personas(int cont1,int cont2)
{
    int distanciaDetectada1;
    int distanciaDetectada2;
    lectura_sensores(distanciaDetectada1,distanciaDetectada2);
    if (distanciaDetectada1 < 1000) 
    { 
        if (cont2 == 0) {
            cont1 = 2;
        } 
        else if (cont2 == 1) {
                cont1 = 1;
            }
    }

    if (distanciaDetectada2 < 1000) 
    {
        if (cont1 == 0) {
            cont2 = 2;
        } 
        else if (cont1 == 1) {
            cont2 = 1;
        }
    }

    if ((cont1 + cont2) > 3) {
        if (cont1 == 2) {
            aforo_actual++;
        }
        else if (cont2 == 2) {
            aforo_actual--;
        }
        cont1 = 0;
        cont2 = 0;
    }
}

void activacion_desactivacion_Alarma()
{
    if (aforo_actual > aforo_maximo && !estado_alarma) {
        alarmaLed = ON;
        alarmaBuzzer = ON;
    } else {
            alarmaLed = OFF;
            alarmaBuzzer = OFF;
        }
    if (Button.read() == 1 && aforo_actual > aforo_maximo) {
        estado_alarma = ON;
    } else if (Button.read() == 1 && estado_alarma) {
        estado_alarma = !estado_alarma;
    }
}

void uart_aforo()
{
    char str[100] = "";
    int stringLength;
    if(aforo_actual > aforo_maximo){
        sprintf ( str, "Aforo: %.d/%.d, aforo maximo superado\r\n", aforo_actual, aforo_maximo);
        stringLength = strlen(str); 
        uartUsb.write( str, stringLength ); 
    }
    else {
        sprintf ( str, "Aforo: %.d/%.d\r\n", aforo_actual, aforo_maximo);
        stringLength = strlen(str); 
        uartUsb.write( str, stringLength ); 
    }
}

int main()
{   int cont1;
    int cont2;
    inputsInit();
    outputsInit();
    while(true){
        deteccion_personas(cont1,cont2);
        activacion_desactivacion_Alarma();
        uart_aforo();
    }
}