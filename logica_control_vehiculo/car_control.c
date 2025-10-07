#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "car_control.h"
#include <my_gpio_lib_rpi4.h>
 
 
// VARIABLES DE ESTADO DEL VEHÍCULO
// =============================
static int batteryLevel = 90;
static int signalStrength = 4;
static int currentSpeed = 0;
static char movementStatus[16] = "stopped";
 
 
// PINES DE CONTROL DE MOTORES
 
#define ENA  13    // PWM para motores izquierdos
#define IN1  19    // Dirección motor izquierdo A
#define IN2  26    // Dirección motor izquierdo B
#define ENB  16    // PWM para motores derechos
#define IN3  20    // Dirección motor derecho A
#define IN4  21    // Dirección motor derecho B
 
static float velocidades[] = {0.0, 0.5, 0.7, 1.0};
static int velocidad_actual = 0; // 0: freno, 1: baja, 2: media, 3: alta
static float freq_pwm = 1000;
 
 
// CONTROL DE PARPADEO DE LUCES (HILOS)
// =============================
typedef struct {
    unsigned int pin1;
    float freq;
    volatile int active;
    pthread_t thread;
} LedBlinkControl;
 
static LedBlinkControl leftBlink = {PIN_LEFT, 2.0, 0, 0};
static LedBlinkControl rightBlink = {PIN_RIGHT, 2.0, 0, 0};
 
void* blink_led_thread(void* arg) {
    LedBlinkControl* ctrl = (LedBlinkControl*)arg;
    float period = 1.0f / ctrl->freq;
    float half_period = period / 2.0f;
    unsigned int half_period_us = (unsigned int)(half_period * 1000000);
    pinMode(ctrl->pin1, OUTPUT);
    while (ctrl->active) {
        digitalWrite(ctrl->pin1, HIGH);
        usleep(half_period_us);
        if (!ctrl->active) break;
        digitalWrite(ctrl->pin1, LOW);
        usleep(half_period_us);
    }
    // Apaga los LEDs al terminar
    digitalWrite(ctrl->pin1, LOW);
    return NULL;
}
 
// Estado interno de las luces
// Estado manual y automático de cada luz
static int headlights_manual = 0, headlights_auto = 0;
static int hazard_manual = 0, hazard_auto = 0;
static int leftSignal_manual = 0, leftSignal_auto = 0;
static int rightSignal_manual = 0, rightSignal_auto = 0;
 
// Devuelve el estado real de la luz (encendida si manual o auto)
static int light_state(int manual, int auto_) { return manual || auto_; }
 
/**
* Controla el estado de las luces del vehículo
*/
// Nuevo: tipo de control para distinguir manual/auto
 
void car_toggle_light(const char* type, int state, LightControlType controlType) {
    // Luces delanteras
    if (strcasecmp(type, "headlights") == 0) {
        if (controlType == LIGHT_MANUAL) {
            headlights_manual = state;
        } else {
            headlights_auto = state;
        }
        int real_state = light_state(headlights_manual, headlights_auto);
        setFrontLeds(PIN_FRONT, real_state);
        printf("[CAR] Luz de adelante: %s (manual=%d, auto=%d)\n", real_state ? "ON" : "OFF", headlights_manual, headlights_auto);
    }
    // Luces traseras
    else if (strcasecmp(type, "hazard") == 0) {
        if (controlType == LIGHT_MANUAL) {
            hazard_manual = state;
        } else {
            hazard_auto = state;
        }
        int real_state = light_state(hazard_manual, hazard_auto);
        setRearLeds(PIN_REAR, real_state);
        printf("[CAR] Luz de atrás: %s (manual=%d, auto=%d)\n", real_state ? "ON" : "OFF", hazard_manual, hazard_auto);
    }
    // Luces direccionales izquierdas (acepta leftSignal y left-signal)
    else if (strcasecmp(type, "leftSignal") == 0 || strcasecmp(type, "left-signal") == 0) {
        if (controlType == LIGHT_MANUAL) {
            leftSignal_manual = state;
        } else {
            leftSignal_auto = state;
        }
        int real_state = light_state(leftSignal_manual, leftSignal_auto);
        if (real_state) {
            if (!leftBlink.active) {
                leftBlink.active = 1;
                pthread_create(&leftBlink.thread, NULL, blink_led_thread, &leftBlink);
            }
        } else {
            if (leftBlink.active) {
                leftBlink.active = 0;
                pthread_join(leftBlink.thread, NULL);
            }
        }
        printf("[CAR] Luz de izquierda: %s (manual=%d, auto=%d)\n", real_state ? "ON" : "OFF", leftSignal_manual, leftSignal_auto);
    }
    // Luces direccionales derechas (acepta rightSignal y right-signal)
    else if (strcasecmp(type, "rightSignal") == 0 || strcasecmp(type, "right-signal") == 0) {
        if (controlType == LIGHT_MANUAL) {
            rightSignal_manual = state;
        } else {
            rightSignal_auto = state;
        }
        int real_state = light_state(rightSignal_manual, rightSignal_auto);
        if (real_state) {
            if (!rightBlink.active) {
                rightBlink.active = 1;
                pthread_create(&rightBlink.thread, NULL, blink_led_thread, &rightBlink);
            }
        } else {
            if (rightBlink.active) {
                rightBlink.active = 0;
                pthread_join(rightBlink.thread, NULL);
            }
        }
        printf("[CAR] Luz de derecha: %s (manual=%d, auto=%d)\n", real_state ? "ON" : "OFF", rightSignal_manual, rightSignal_auto);
    }
    // Otros tipos de luces no implementados
    else {
        printf("[CAR] Luz %s: %s\n", type, state ? "ON" : "OFF");
    }
}
 
// FUNCIONES DE MOVIMIENTO (L298N)
// =============================
void car_move_forward() {
    // Encender luces delanteras automáticamente
    if (!headlights_auto) {
        car_toggle_light("headlights", 1, LIGHT_AUTO);
    }
    printf("[CAR] Ejecutando: ADELANTE | Duty: %.2f | Freq: %.0f Hz\n", velocidades[velocidad_actual], freq_pwm);
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    // Motores izquierdos adelante
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    // Motores derechos adelante
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    if (!pwm_is_running()) {
        pwm_start(ENA, ENB, velocidades[velocidad_actual], freq_pwm);
    } else {
        pwm_update(velocidades[velocidad_actual], freq_pwm);
    }
}
 
// Función para mover el coche hacia atrás
void car_move_backward() {
    // Encender luces traseras automáticamente
    if (!hazard_auto) {
        car_toggle_light("hazard", 1, LIGHT_AUTO);
    }
    printf("[CAR] Ejecutando: ATRÁS | Duty: %.2f | Freq: %.0f Hz\n", velocidades[velocidad_actual], freq_pwm);
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    // Motores izquierdos atrás
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    // Motores derechos atrás
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    if (!pwm_is_running()) {
        pwm_start(ENA, ENB, velocidades[velocidad_actual], freq_pwm);
    } else {
        pwm_update(velocidades[velocidad_actual], freq_pwm);
    }
}
 
// Función para girar el coche a la izquierda
void car_turn_left() {
    // Encender direccional izquierda automáticamente
    if (!leftSignal_auto) {
        car_toggle_light("leftSignal", 1, LIGHT_AUTO);
    }
    printf("[CAR] Ejecutando: IZQUIERDA | Duty: %.2f | Freq: %.0f Hz\n", velocidades[velocidad_actual], freq_pwm);
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    // Motores izquierdos adelante
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    // Motores derechos atrás
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    if (!pwm_is_running()) {
        pwm_start(ENA, ENB, velocidades[velocidad_actual], freq_pwm);
    } else {
        pwm_update(velocidades[velocidad_actual], freq_pwm);
    }
}
 
// Función para girar el coche a la derecha
void car_turn_right() {
    // Encender direccional derecha automáticamente
    if (!rightSignal_auto) {
        car_toggle_light("rightSignal", 1, LIGHT_AUTO);
    }
    printf("[CAR] Ejecutando: DERECHA | Duty: %.2f | Freq: %.0f Hz\n", velocidades[velocidad_actual], freq_pwm);
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    // Motores izquierdos atrás
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    // Motores derechos adelante
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    if (!pwm_is_running()) {
        pwm_start(ENA, ENB, velocidades[velocidad_actual], freq_pwm);
    } else {
        pwm_update(velocidades[velocidad_actual], freq_pwm);
    }
}
 
// Función para detener el coche
void car_stop() {
    // Apagar luces automáticas (mantener encendidas si el usuario las dejó manualmente)
    if (headlights_auto) {
        car_toggle_light("headlights", 0, LIGHT_AUTO);
    }
    if (hazard_auto) {
        car_toggle_light("hazard", 0, LIGHT_AUTO);
    }
    if (leftSignal_auto) {
        car_toggle_light("leftSignal", 0, LIGHT_AUTO);
    }
    if (rightSignal_auto) {
        car_toggle_light("rightSignal", 0, LIGHT_AUTO);
    }
    printf("[CAR] Ejecutando: STOP\n");
    pwm_stop();
    // Opcional: poner todos los pines de dirección en LOW para freno activo
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
 
 
// FUNCIONES DE ESTADO Y MOVIMIENTO
// =============================
void car_get_status(char* buffer, int buflen) {
    snprintf(buffer, buflen,
        "{\"batteryLevel\":%d,\"signalStrength\":%d,\"currentSpeed\":%d,\"movementStatus\":\"%s\"}",
        batteryLevel, signalStrength, currentSpeed, movementStatus);
}
 
 
void car_set_direction(const char* direction) {
    printf("[CAR] Dirección: %s\n", direction);
    strncpy(movementStatus, direction, sizeof(movementStatus));
    movementStatus[sizeof(movementStatus)-1] = '\0';
 
    if (strcmp(direction, "forward") == 0) {
        car_move_forward();
    } else if (strcmp(direction, "backward") == 0) {
        car_move_backward();
    } else if (strcmp(direction, "left") == 0) {
        car_turn_left();
    } else if (strcmp(direction, "right") == 0) {
        car_turn_right();
    } else if (strcmp(direction, "stop") == 0) {
        car_stop();
    }
}
 
void car_set_speed(int speed) {
    printf("[CAR] Velocidad: %d\n", speed);
    currentSpeed = speed;
    if (speed >= 0 && speed <= 3) {
        velocidad_actual = speed;
        if (pwm_is_running()) {
            pwm_update(velocidades[velocidad_actual], freq_pwm);
        }
    }
}