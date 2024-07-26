#include "mbed.h"
#include "arm_book_lib.h"

#define TIEMPO_PARPADEO_ALARMA 10
#define FACTOR_ESCALA 614.4 // factor de escala del sensor US-016 en mm/V 
#define UMBRAL_DETECCION 1000 // distancia en mm de deteccion

DigitalIn Button(D2);
AnalogIn sensorDeteccion1(A0);
AnalogIn sensorDeteccion2(A1);

DigitalOut alarmaLed(D0);
DigitalOut alarmaBuzzer(D1);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int aforoActual = 0;
int aforoMaximo = 40;
bool alarmaActivada;
bool alarmaPausada;

void inputsInit();
void outputsInit();
void lectura_sensores();
void deteccion_personas(int *distanciaDetectada1, int *distanciaDetectada2);
void activacion_desactivacion_Alarma(int *cont1, int *cont2);
void uart_aforo();

void inputsInit()
{
    Button.mode(PullDown);
}

void outputsInit()
{
    alarmaActivada = false;
    alarmaPausada = false;
    alarmaLed = OFF;
    alarmaBuzzer = OFF;
}

void lectura_sensores(int *distanciaDetectada1, int *distanciaDetectada2)
{
    *distanciaDetectada1 = ((sensorDeteccion1.read() * 3.3) * FACTOR_ESCALA);
    *distanciaDetectada2 = ((sensorDeteccion2.read() * 3.3) * FACTOR_ESCALA);
}

void deteccion_personas(int *cont1, int *cont2)
{
    int distanciaDetectada1;
    int distanciaDetectada2;
    lectura_sensores(&distanciaDetectada1, &distanciaDetectada2);

    if (distanciaDetectada1 < UMBRAL_DETECCION) 
    {
        if (*cont2 == 0) {
            *cont1 = 2;
        } 
        else if (*cont2 == 1) {
            *cont1 = 1;
        }
    }

    if (distanciaDetectada2 < UMBRAL_DETECCION) 
    {
        if (*cont1 == 0) {
            *cont2 = 2;
        } 
        else if (*cont1 == 1) {
            *cont2 = 1;
        }
    }

    if ((*cont1 + *cont2) > 3) {
        if (*cont1 == 2) {
            aforoActual++;
        }
        else if (*cont2 == 2) {
            aforoActual--;
        }
        *cont1 = 0;
        *cont2 = 0;
    }
}

// Al presionar el botón, la alarma se desactiva y sigue sensando.
// Si se presiona nuevamente, la alarma se reactivará si el aforo sigue superado.

void manejarAlarma() {
    if (aforoActual > aforoMaximo && !alarmaPausada) {
        alarmaActivada = true;
        alarmaLed = ON;
        alarmaBuzzer = ON;
        delay(TIEMPO_PARPADEO_ALARMA);
        alarmaLed = OFF;
        alarmaBuzzer = OFF;
    } else if (aforoActual <= aforoMaximo && alarmaPausada) {
        alarmaActivada = false;
        alarmaLed = OFF;
        alarmaBuzzer = OFF;
    }

    if (Button.read() == 1) {
        alarmaPausada = !alarmaPausada;
    }
}

void uart_aforo()
{
    char str[150] = "";
    int stringLength;
    
    if (aforoActual > aforoMaximo) {
        stringLength = sprintf(str, "Aforo: %d/%d, aforo maximo superado\r\n", aforoActual, aforoMaximo);
    } else {
        stringLength = sprintf(str, "Aforo: %d/%d\r\n", aforoActual, aforoMaximo);
    }
    
    if (alarmaPausada) {
        stringLength += sprintf(str + stringLength, "Alarma en pausa\r\n");
    }
    
    uartUsb.write(str, stringLength); 
}

int main()
{   int cont1;
    int cont2;
    inputsInit();
    outputsInit();
    while(true){
        deteccion_personas(&cont1, &cont2);
        manejarAlarma();
        uart_aforo();
    }
}