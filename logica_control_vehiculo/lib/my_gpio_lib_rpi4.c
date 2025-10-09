// ============================================================================
// my_gpio_lib_rpi4.c
// Biblioteca de control GPIO para Raspberry Pi 4 (BCM2711)
// ============================================================================

#include "my_gpio_lib_rpi4.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

// -----------------------------------------------------------------------------
// 1. DEFINICIONES Y CONFIGURACIÓN DE HARDWARE
// -----------------------------------------------------------------------------

#define RPI4_PERIPH_BASE    0xFE000000 // Dirección base de periféricos para RPi4
#define GPIO_BASE           (RPI4_PERIPH_BASE + 0x200000) // Base GPIO
#define GPIO_LEN            0xF4

// Registros GPIO
#define GPSET0 7    // Set GPIO pins 0-31
#define GPSET1 8    // Set GPIO pins 32-53
#define GPCLR0 10   // Clear GPIO pins 0-31
#define GPCLR1 11   // Clear GPIO pins 32-53
#define GPLEV0 13   // Read GPIO pins 0-31
#define GPLEV1 14   // Read GPIO pins 32-53

// Puntero a registros GPIO mapeados en memoria
static volatile uint32_t *gpioReg = MAP_FAILED;

// Macros para banco y bit
#define PI_BANK (pin>>5)
#define PI_BIT  (1<<(pin&0x1F))

// Variable para almacenar información de hardware
static unsigned int hardware_revision = 0;

// Variables para PWM controlado por hilo
static pthread_t pwm_thread;
static int pwm_running = 0;
static float pwm_duty = 0.0f;
static float pwm_freq = 0.0f;
static unsigned int pwm_pin1 = 0, pwm_pin2 = 0;
static pthread_mutex_t pwm_mutex = PTHREAD_MUTEX_INITIALIZER;

// -----------------------------------------------------------------------------
// 2. UTILIDADES DE DETECCIÓN Y MAPEO DE HARDWARE
// -----------------------------------------------------------------------------


// Verifica que estemos ejecutando en Raspberry Pi 4
static int verifyRaspberryPi4(void) {
    FILE *filp;
    char buf[512];
    uint32_t detected_base = 0;

    // Verifica la dirección base desde el device tree
    filp = fopen("/proc/device-tree/soc/ranges", "rb");
    if (filp != NULL) {
        if (fread(buf, 1, sizeof(buf), filp) >= 8) {
            detected_base = buf[4]<<24 | buf[5]<<16 | buf[6]<<8 | buf[7];
            if (!detected_base)
                detected_base = buf[8]<<24 | buf[9]<<16 | buf[10]<<8 | buf[11];
        }
        fclose(filp);
    }

    // Verifica que sea Raspberry Pi 4
    if (detected_base != RPI4_PERIPH_BASE) {
        fprintf(stderr, "ERROR: Esta biblioteca está optimizada SOLO para Raspberry Pi 4!\n");
        fprintf(stderr, "       Dirección detectada: 0x%08X, esperada: 0x%08X\n", 
                detected_base, RPI4_PERIPH_BASE);
        fprintf(stderr, "       Para otros modelos, usa my_gpio_lib.h\n");
        return -1;
    }

    return 0;
}


// Obtiene la revisión del hardware
static unsigned int getHardwareRevision(void) {
    FILE *filp;
    char buf[512];
    char term;
    unsigned int rev = 0;
    int chars = 4;

    filp = fopen("/proc/cpuinfo", "r");
    if (filp != NULL) {
        while (fgets(buf, sizeof(buf), filp) != NULL) {
            if (!strncasecmp("revision", buf, 8)) {
                if (sscanf(buf + strlen(buf) - (chars + 1), "%x%c", &rev, &term) == 2) {
                    if (term != '\n') rev = 0;
                    else rev &= 0xFFFFFF;
                }
            }
        }
        fclose(filp);
    }

    return rev;
}


// Mapea una región de memoria física
static uint32_t* mapMemory(int fd, uint32_t addr, uint32_t len) {
    return (uint32_t*)mmap(0, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);
}


// Obtiene información del hardware
unsigned int getHardwareInfo(void) {
    return hardware_revision;
}



// -----------------------------------------------------------------------------
// 3. INICIALIZACIÓN Y LIMPIEZA DE LA BIBLIOTECA
// -----------------------------------------------------------------------------


// Inicializa la biblioteca GPIO para Raspberry Pi 4
int gpioInit(void) {
    int fd;

    printf("Inicializando GPIO para Raspberry Pi 4 (BCM2711)...\n");

    // Verifica que estemos en Pi 4
    if (verifyRaspberryPi4() < 0) {
        return -1;
    }

    // Obtiene información del hardware
    hardware_revision = getHardwareRevision();
    printf("Revisión de hardware detectada: 0x%06X\n", hardware_revision);

    // Abre /dev/mem para acceso directo a memoria
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Error: Esta biblioteca necesita permisos root. Usa sudo\n");
        return -1;
    }

    // Mapea los registros GPIO en memoria
    gpioReg = mapMemory(fd, GPIO_BASE, GPIO_LEN);
    close(fd);

    if (gpioReg == MAP_FAILED) {
        fprintf(stderr, "Error: Fallo al mapear registros GPIO\n");
        return -1;
    }

    printf("GPIO inicializado exitosamente para Raspberry Pi 4\n");
    return 0;
}


// Limpia los recursos utilizados
void gpioCleanup(void) {
    if (gpioReg != MAP_FAILED) {
        munmap((void*)gpioReg, GPIO_LEN);
        gpioReg = MAP_FAILED;
        printf("Recursos GPIO liberados\n");
    }
}



// -----------------------------------------------------------------------------
// 4. FUNCIONES BÁSICAS DE CONTROL GPIO
// -----------------------------------------------------------------------------


// Establece el modo de un pin GPIO
void pinMode(unsigned int pin, unsigned int mode) {
    int reg, shift;

    if (pin > 53) {
        fprintf(stderr, "Error: Pin %d fuera de rango (0-53)\n", pin);
        return;
    }

    reg = pin / 10;
    shift = (pin % 10) * 3;

    // Solo soportamos INPUT (0) y OUTPUT (1)
    if (mode > 1) mode = 1;

    gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}


// Lee el estado digital de un pin GPIO
int digitalRead(unsigned int pin) {
    if (pin > 53) {
        fprintf(stderr, "Error: Pin %d fuera de rango (0-53)\n", pin);
        return 0;
    }

    if ((*(gpioReg + GPLEV0 + PI_BANK) & PI_BIT) != 0) {
        return HIGH;
    } else {
        return LOW;
    }
}


// Escribe un valor digital en un pin GPIO
void digitalWrite(unsigned int pin, unsigned int value) {
    if (pin > 53) {
        fprintf(stderr, "Error: Pin %d fuera de rango (0-53)\n", pin);
        return;
    }

    if (value == LOW) {
        *(gpioReg + GPCLR0 + PI_BANK) = PI_BIT;
    } else {
        *(gpioReg + GPSET0 + PI_BANK) = PI_BIT;
    }
}



// -----------------------------------------------------------------------------
// 5. FUNCIONES DE USUARIO: BLINK Y PWM
// -----------------------------------------------------------------------------


// Genera un parpadeo en un pin GPIO
void blink(unsigned int pin, float freq, float duration) {
    if (pin > 53) {
        fprintf(stderr, "Error: Pin %d fuera de rango (0-53)\n", pin);
        return;
    }
    
    if (freq <= 0 || duration <= 0) {
        fprintf(stderr, "Error: Frecuencia y duración deben ser positivos\n");
        return;
    }

    // Calcula el período y tiempo de cada estado
    float period = 1.0 / freq;           // Período completo en segundos
    float half_period = period / 2.0;    // Tiempo en alto y bajo
    
    // Convierte a microsegundos para usleep
    unsigned int half_period_us = (unsigned int)(half_period * 1000000);
    
    // Calcula el número total de ciclos
    int total_cycles = (int)(duration * freq);
    
    printf("Iniciando blink en pin %d: %.1f Hz por %.1f segundos (%d ciclos)\n", 
           pin, freq, duration, total_cycles);
    
    // Configura el pin como salida
    pinMode(pin, OUTPUT);
    
    // Ejecuta el parpadeo
    for (int i = 0; i < total_cycles; i++) {
        digitalWrite(pin, HIGH);
        usleep(half_period_us);
        digitalWrite(pin, LOW);
        usleep(half_period_us);
    }
    
    printf("Blink completado en pin %d\n", pin);
}


// Genera PWM por software en dos pines en paralelo
void PWM(unsigned int pin1, unsigned int pin2, float duty_cycle, float freq, float duration) {
    if (pin1 > 53 || pin2 > 53) {
        return;
    }
    if (duty_cycle < 0.0) {
        duty_cycle = 0.0;
    }
    if (duty_cycle > 1.0) {
        duty_cycle = 1.0;
    }

    float period = 1.0 / freq;
    float high_time = period * duty_cycle;
    float low_time  = period - high_time;

    unsigned int high_us = (unsigned int)(high_time * 1e6);
    unsigned int low_us  = (unsigned int)(low_time  * 1e6);

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);

    long cycles = (duration > 0) ? (long)(freq * duration) : -1;

    while (cycles != 0) {
        if (high_us > 0) {
            digitalWrite(pin1, HIGH);
            digitalWrite(pin2, HIGH);
            usleep(high_us);
        }
        if (low_us > 0) {
            digitalWrite(pin1, LOW);
            digitalWrite(pin2, LOW);
            usleep(low_us);
        }
        if (cycles > 0) cycles--;
    }
}



// -----------------------------------------------------------------------------
// 6. FUNCIONES DE CONTROL DE LEDS DEL VEHÍCULO
// -----------------------------------------------------------------------------


void setFrontLeds(unsigned int pin, int state) {
    pinMode(pin, OUTPUT);
    if (state == 1) {
        digitalWrite(pin, HIGH);
    } else if (state == 0) {
        digitalWrite(pin, LOW);
    }
}


void setRearLeds(unsigned int pin, int state) {
    pinMode(pin, OUTPUT);
    if (state == 1) {
        digitalWrite(pin, HIGH);
    } else if (state == 0) {
        digitalWrite(pin, LOW);
    }
}


// PWM controlado por hilo (continua hasta que se detenga)

static void* pwm_thread_func(void* arg) {
    pinMode(pwm_pin1, OUTPUT);
    pinMode(pwm_pin2, OUTPUT);
    while (1) {
        pthread_mutex_lock(&pwm_mutex);
        int running = pwm_running;
        float duty = pwm_duty;
        float freq = pwm_freq;
        pthread_mutex_unlock(&pwm_mutex);
        if (!running) break;
        if (freq <= 0.0f) freq = 1.0f;
        if (duty < 0.0f) duty = 0.0f;
        if (duty > 1.0f) duty = 1.0f;
        float period = 1.0f / freq;
        float high_time = period * duty;
        float low_time = period - high_time;
        unsigned int high_us = (unsigned int)(high_time * 1e6f);
        unsigned int low_us  = (unsigned int)(low_time  * 1e6f);
        if (high_us > 0) {
            digitalWrite(pwm_pin1, HIGH);
            digitalWrite(pwm_pin2, HIGH);
            usleep(high_us);
        }
        if (low_us > 0) {
            digitalWrite(pwm_pin1, LOW);
            digitalWrite(pwm_pin2, LOW);
            usleep(low_us);
        }
    }
    // Apaga los pines al terminar
    digitalWrite(pwm_pin1, LOW);
    digitalWrite(pwm_pin2, LOW);
    return NULL;
}


// -----------------------------------------------------------------------------
// PWM CONTROLADO POR HILO (CONTINUO)
// -----------------------------------------------------------------------------


void pwm_start(unsigned int pin1, unsigned int pin2, float duty_cycle, float freq) {
    pthread_mutex_lock(&pwm_mutex);
    if (pwm_running) {
        pthread_mutex_unlock(&pwm_mutex);
        pwm_stop();
        pthread_mutex_lock(&pwm_mutex);
    }
    pwm_pin1 = pin1;
    pwm_pin2 = pin2;
    pwm_duty = duty_cycle;
    pwm_freq = freq;
    pwm_running = 1;
    pthread_mutex_unlock(&pwm_mutex);
    pthread_create(&pwm_thread, NULL, pwm_thread_func, NULL);
}

void pwm_update(float duty_cycle, float freq) {
    pthread_mutex_lock(&pwm_mutex);
    pwm_duty = duty_cycle;
    pwm_freq = freq;
    pthread_mutex_unlock(&pwm_mutex);
}

void pwm_stop(void) {
    pthread_mutex_lock(&pwm_mutex);
    int was_running = pwm_running;
    pwm_running = 0;
    pthread_mutex_unlock(&pwm_mutex);
    if (was_running) {
        pthread_join(pwm_thread, NULL);
    }
}

int pwm_is_running(void) {
    pthread_mutex_lock(&pwm_mutex);
    int running = pwm_running;
    pthread_mutex_unlock(&pwm_mutex);
    return running;
}