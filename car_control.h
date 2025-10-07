
#ifndef CAR_CONTROL_H
#define CAR_CONTROL_H

// Pines GPIO de ejemplo para cada luz
#define PIN_FRONT1  4   // fisico 7
#define PIN_FRONT2  14  // fisico 8

#define PIN_RIGHT1  15  // fisico 10
#define PIN_RIGHT2  18  // fisico 12

#define PIN_LEFT1   17  // fisico 11
#define PIN_LEFT2   27  // fisico 13

#define PIN_REAR1   22  // fisico 15
#define PIN_REAR2   23  // fisico 16

// Funciones para controlar el carrito
void car_set_direction(const char* direction);
void car_set_speed(int speed);
void car_toggle_light(const char* type, int state);
void car_get_status(char* buffer, int buflen);

#endif // CAR_CONTROL_H
