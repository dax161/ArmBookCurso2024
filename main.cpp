#include "mbed.h"
#include "arm_book_lib.h"

#define TIEMPO_PARPADEO_ALARMA 10
#define FACTOR_ESCALA 614.4 // Factor de escala del sensor US-016 en mm/V 
#define UMBRAL_DETECCION 1000 // Distancia en mm de detección

DigitalIn button(D2);
AnalogIn sensorEntrada(A0);
AnalogIn sensorSalida(A1);

DigitalOut alarmaLed(D0);
DigitalOut alarmaBuzzer(D1);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int aforoActual = 0;
int aforoMaximo = 40;
bool alarmaActivada;
bool alarmaPausada;

void inicializarEntradas();
void inicializarSalidas();
void leerSensores(int* distanciaEntrada, int* distanciaSalida);
void detectarPersonas(int* contadorEntrada, int* contadorSalida);
void manejarAlarma();
void enviarAforoPorUart();

void inicializarEntradas() {
    button.mode(PullDown);
}

void inicializarSalidas() {
    alarmaActivada = false;
    alarmaPausada = false;
    alarmaLed = OFF;
    alarmaBuzzer = OFF;
}

void leerSensores(int* distanciaEntrada, int* distanciaSalida) {
    *distanciaEntrada = (sensorEntrada.read() * 3.3) * FACTOR_ESCALA;
    *distanciaSalida = (sensorSalida.read() * 3.3) * FACTOR_ESCALA;
}

void detectarPersonas(int* contadorEntrada, int* contadorSalida) {
    int distanciaEntrada;
    int distanciaSalida;
    
    leerSensores(&distanciaEntrada, &distanciaSalida);

    if (distanciaEntrada < UMBRAL_DETECCION) {
        if (*contadorSalida == 0) {
            *contadorEntrada = 2;
        } else if (*contadorSalida == 1) {
            *contadorEntrada = 1;
        }
    }

    if (distanciaSalida < UMBRAL_DETECCION) {
        if (*contadorEntrada == 0) {
            *contadorSalida = 2;
        } else if (*contadorEntrada == 1) {
            *contadorSalida = 1;
        }
    }

    if ((*contadorEntrada + *contadorSalida) > 3) {
        if (*contadorEntrada == 2) {
            aforoActual++;
        } else if (*contadorSalida == 2) {
            aforoActual--;
        }
        *contadorEntrada = 0;
        *contadorSalida = 0;
    }
}

void manejarAlarma() {
    if (aforoActual > aforoMaximo && !alarmaPausada) {
        alarmaActivada = true;
        alarmaLed = ON;
        alarmaBuzzer = ON;
        delay(TIEMPO_PARPADEO_ALARMA);
        alarmaLed = OFF;
        alarmaBuzzer = OFF;
    } else if (aforoActual <= aforoMaximo) {
        alarmaActivada = false;
        alarmaLed = OFF;
        alarmaBuzzer = OFF;
    }

    if (button.read() == 1) {
        alarmaPausada = !alarmaPausada;
    }
}

void enviarAforoPorUart() {
    char mensaje[150] = "";
    int longitudMensaje;

    if (aforoActual > aforoMaximo) {
        longitudMensaje = sprintf(mensaje, "Aforo: %d/%d, aforo maximo superado\r\n", aforoActual, aforoMaximo);
    } else {
        longitudMensaje = sprintf(mensaje, "Aforo: %d/%d\r\n", aforoActual, aforoMaximo);
    }

    if (alarmaPausada) {
        longitudMensaje += sprintf(mensaje + longitudMensaje, "Alarma en pausa\r\n");
    }

    uartUsb.write(mensaje, longitudMensaje);
}

int main() {
    int contadorEntrada = 0;
    int contadorSalida = 0;

    inicializarEntradas();
    inicializarSalidas();

    while (true) {
        detectarPersonas(&contadorEntrada, &contadorSalida);
        manejarAlarma();
        enviarAforoPorUart();
    }
}