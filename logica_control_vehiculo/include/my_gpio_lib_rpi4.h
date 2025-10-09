#ifndef MY_GPIO_LIB_RPI4_H
#define MY_GPIO_LIB_RPI4_H

#include <stdint.h>

/* GPIO Modes */
#define INPUT  0
#define OUTPUT 1

/* Digital Values */
#define LOW  0
#define HIGH 1

/*
 * Inicializa la biblioteca GPIO para Raspberry Pi 4
 */
int gpioInit(void);

/*
 * Establece el modo de un pin GPIO
 */
void pinMode(unsigned int pin, unsigned int mode);

/*
 * Escribe un valor digital en un pin GPIO configurado como salida
 */
void digitalWrite(unsigned int pin, unsigned int value);

/*
 * Lee el estado digital de un pin GPIO
 */
int digitalRead(unsigned int pin);

/*
 * Genera un parpadeo en un pin GPIO
 */
void blink(unsigned int pin, float freq, float duration);

/*
 * Limpia los recursos utilizados por la biblioteca
 */
void gpioCleanup(void);

/*
 * Obtiene información del hardware (solo para debug)
 */
unsigned int getHardwareInfo(void);


/*
 * PWM controlado por hilo (continua hasta que se detenga)
 */
void pwm_start(unsigned int pin1, unsigned int pin2, float duty_cycle, float freq);
void pwm_update(float duty_cycle, float freq);
void pwm_stop(void);
int pwm_is_running(void);

/*
 * Genera PWM por software en dos pines en paralelo (legacy)
 */
void PWM(unsigned int pin1, unsigned int pin2, float duty_cycle, float freq, float duration);

/*
 * Funciones para encender y apagar LEDs de un carro de control remoto
 * Los pines se pasan como parámetros para flexibilidad
 */
void setFrontLeds(unsigned int pin, int state);
void setRearLeds(unsigned int pin, int state);

#endif /* MY_GPIO_LIB_RPI4_H */