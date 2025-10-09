#include "my_gpio_lib_rpi4.h"
#include <stdio.h>
#include <unistd.h>

// Motor A (izquierdo)
#define ENA  13    // fisico 33 (pwm1)
#define IN1  19    // fisico 35
#define IN2  26    // fisico 37

// Motor B (derecho)
#define ENB  16   // fisico 36
#define IN3  20   // fisico 38
#define IN4  21   // fisico 40

#define STBY 12   // fisico 32 (pwm0)

int main() {
    if (gpioInit() < 0) return -1;

    // Configurar pin STBY y activarlo
    pinMode(STBY, OUTPUT);
    digitalWrite(STBY, HIGH); // Activa el TB6612FNG

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

    // Prueba de varias velocidades hacia adelante
    float velocidades[] = {0.3, 0.6, 1.0};
    int n = sizeof(velocidades)/sizeof(velocidades[0]);
    printf("Prueba de varias velocidades (adelante):\n");
    for (int i = 0; i < n; i++) {
        printf("  Duty cycle: %.1f%%\n", velocidades[i]*100);
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Motor A adelante
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Motor B adelante
        PWM(ENA, ENB, velocidades[i], 1000, 3);
        usleep(500000); // Pausa de 0.5s entre pruebas
    }

    gpioCleanup();
    return 0;
}