
#ifndef CAR_CONTROL_H
#define CAR_CONTROL_H

// Pines GPIO de ejemplo para cada luz
#define PIN_FRONT  14  // fisico 8

#define PIN_RIGHT  18  // fisico 12

#define PIN_LEFT   27  // fisico 13

#define PIN_REAR   23  // fisico 16

// Funciones para controlar el carrito
void car_set_direction(const char* direction);
void car_set_speed(int speed);
typedef enum { LIGHT_MANUAL, LIGHT_AUTO } LightControlType;
void car_toggle_light(const char* type, int state, LightControlType controlType);
void car_get_status(char* buffer, int buflen);

#endif // CAR_CONTROL_H
