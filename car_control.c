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



// CONTROL DE PARPADEO DE LUCES (HILOS)
// =============================
typedef struct {
    unsigned int pin1;
    unsigned int pin2;
    float freq;
    float duration;
    volatile int active;
    pthread_t thread;
} LedBlinkControl;

static LedBlinkControl leftBlink = {PIN_LEFT1, PIN_LEFT2, 2.0, 1.5, 0, 0};
static LedBlinkControl rightBlink = {PIN_RIGHT1, PIN_RIGHT2, 2.0, 1.5, 0, 0};

void* blink_led_thread(void* arg) {
    LedBlinkControl* ctrl = (LedBlinkControl*)arg;
    float period = 1.0f / ctrl->freq;
    float half_period = period / 2.0f;
    unsigned int half_period_us = (unsigned int)(half_period * 1000000);
    pinMode(ctrl->pin1, OUTPUT);
    pinMode(ctrl->pin2, OUTPUT);
    while (ctrl->active) {
        digitalWrite(ctrl->pin1, HIGH);
        digitalWrite(ctrl->pin2, HIGH);
        usleep(half_period_us);
        if (!ctrl->active) break;
        digitalWrite(ctrl->pin1, LOW);
        digitalWrite(ctrl->pin2, LOW);
        usleep(half_period_us);
    }
    // Apaga los LEDs al terminar
    digitalWrite(ctrl->pin1, LOW);
    digitalWrite(ctrl->pin2, LOW);
    return NULL;
}

// Estado interno de las luces
static int headlights = 0;
static int hazard = 0;
static int leftSignal = 0;
static int rightSignal = 0;

/**
 * Controla el estado de las luces del vehículo
 */
void car_toggle_light(const char* type, int state) {
    
    // Luces delanteras
    if (strcmp(type, "headlights") == 0) {
        headlights = state;
        setFrontLeds(PIN_FRONT1, PIN_FRONT2, state);
        printf("[CAR] Luz de adelante: %s\n", headlights ? "ON" : "OFF");
    }
    
    // Luces traseras
    else if (strcmp(type, "hazard") == 0) {
        hazard = state;
        setRearLeds(PIN_REAR1, PIN_REAR2, state);
        printf("[CAR] Luz de atrás: %s\n", hazard ? "ON" : "OFF");
    }

    // Luces izquierdas
    else if (strcmp(type, "leftSignal") == 0) {
        leftSignal = state;
        if (state) {
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
        printf("[CAR] Luz de izquierda: %s\n", leftSignal ? "ON" : "OFF");
    }

    // Luces derechas
    else if (strcmp(type, "rightSignal") == 0) {
        rightSignal = state;
        if (state) {
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
        printf("[CAR] Luz de derecha: %s\n", rightSignal ? "ON" : "OFF");
    }
    
    // Otros tipos de luces no implementados
    else {
        printf("[CAR] Luz %s: %s\n", type, state ? "ON" : "OFF");
    }
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
}

void car_set_speed(int speed) {
    printf("[CAR] Velocidad: %d\n", speed);
    currentSpeed = speed;
}
