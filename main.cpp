#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn Button(D2);

AnalogIn sensor1(A0);
AnalogIn sensor2(A1);

DigitalOut alarmLed(D0);
DigitalOut alarmBuzzer(D1);

int aforo_actual = 0;
int aforo_maximo = 30;
bool alarmSate = OFF;

UnbufferedSerial uartUsb(USBTX, USBRX, 9600);

void inputsInit()
{
    Button.mode(PullDown);
}

void outputsInit()
{
    alarmSate = OFF;
    alarmLed = OFF;
    alarmBuzzer = OFF;
}

void imprimirAforo(int aforo_actual,int aforo_maximo)
{
    char str[30];
    sprintf ( str, "Aforo: %d/%d\r\n", aforo_actual,aforo_maximo );
    uartUsb.write( str, strlen(str) );
}

int procesarSensores(int aforo_actual)
{
    int cont1 = 0;
    int cont2 = 0;
    float sensor1Reading = 0;
    float sensor2Reading = 0;
    bool sensores_activados = false;

    while (!sensores_activados)
    {
        sensor1Reading = sensor1.read() * 3.3; // Convierte la lectura a voltaje
        sensor2Reading = sensor2.read() * 3.3; // Convierte la lectura a voltaje

        if (sensor1Reading > 2.0) { // Ajusta el umbral según tu sensor
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

            // Reiniciar los contadores
            cont1 = 0;
            cont2 = 0;

            // Marcar que los sensores fueron activados correctamente
            sensores_activados = true;
        }

        // Pequeño retraso para evitar lecturas excesivas
        delay(200);
    }

    return aforo_actual;
}

bool activacion_desactivacion_Alarma(int aforo_actual, int aforo_maximo)
{
    while (aforo_actual > aforo_maximo)
    {
        alarmLed = ON;
        alarmBuzzer = ON;
        if (Button.read() == 1) {
            alarmLed = OFF;
            alarmBuzzer = OFF;
            return false; // Indicar que el sistema debe detenerse
        }
    }

    return true; // Continuar el sistema
}

int main()
{   
    inputsInit();
    outputsInit();

    while(!alarmSate)
    {
        aforo_actual = procesarSensores(aforo_actual);
        imprimirAforo(aforo_actual, aforo_maximo);
        if (aforo_actual > aforo_maximo) {
            alarmSate = activacion_desactivacion_Alarma(aforo_actual, aforo_maximo);
        }
    }
}
