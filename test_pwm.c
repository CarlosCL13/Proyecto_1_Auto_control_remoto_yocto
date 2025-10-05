#include "my_gpio_lib_rpi4.h"
#include <stdio.h>
#include <unistd.h>

// Motor A (izquierdo)
#define ENA  12   // GPIO12 → EnableA  en fisico 32
#define IN1  5    // GPIO5
#define IN2  6    // GPIO6

// Motor B (derecho)
#define ENB  13   // GPIO13 → EnableB  en fisico 33
#define IN3  19   // GPIO19
#define IN4  26   // GPIO26

int main() {
    if (gpioInit() < 0) return -1;

    // Configurar pines de dirección
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Adelante
    printf("Adelante\n");
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Motor A adelante
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Motor B adelante
    PWM(ENA, ENB, 0.7, 1000, 3);                 // Ambos motores al mismo tiempo

    // Atrás
    printf("Atrás\n");
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);   // Motor A atrás
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);   // Motor B atrás
    PWM(ENA, ENB, 0.7, 1000, 3);

    // Derecha
    printf("Derecha\n");
    digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // Motor A atrás
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Motor B adelante
    PWM(ENA, ENB, 0.7, 1000, 2);

    // Izquierda
    printf("Izquierda\n");
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Motor A adelante
    digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // Motor B atrás
    PWM(ENA, ENB, 0.7, 1000, 2);

    gpioCleanup();
    return 0;
}
