#include "my_gpio_lib_rpi4.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("========================================\n");
    printf("  Test de Biblioteca GPIO para Pi 4\n");
    printf("========================================\n");
    
    // Inicializar la biblioteca GPIO
    if (gpioInit() < 0) {
        printf("ERROR: No se pudo inicializar GPIO\n");
        printf("Verifica que:\n");
        printf("1. Estés ejecutando en Raspberry Pi 4\n");
        printf("2. Tengas permisos root (sudo)\n");
        return 1;
    }
    
    // Mostrar información del hardware
    printf("Información del hardware: 0x%06X\n", getHardwareInfo());
    printf("\n");
    
    // Definir pines para las pruebas
    int led_pin = 2;    // Pin GPIO 2 (físico 3)
    int button_pin = 3; // Pin GPIO 3 (físico 5)
    
    // Configurar pines
    printf("Configurando pines...\n");
    pinMode(led_pin, OUTPUT);
    pinMode(button_pin, INPUT);
    
    printf("✓ Pin %d configurado como salida (LED)\n", led_pin);
    printf("✓ Pin %d configurado como entrada (BOTÓN)\n", button_pin);
    printf("\n");
    
    // Test 1: digitalWrite básico
    printf("--- Test 1: Control básico de LED ---\n");
    printf("Encendiendo LED en pin %d por 2 segundos...\n", led_pin);
    digitalWrite(led_pin, HIGH);
    sleep(2);
    
    printf("Apagando LED por 1 segundo...\n");
    digitalWrite(led_pin, LOW);
    sleep(1);
    printf("✓ Test 1 completado\n\n");
    
    // Test 2: digitalRead
    printf("--- Test 2: Lectura de estado de pines ---\n");
    printf("Estado del pin %d (botón): %s\n", button_pin, 
           digitalRead(button_pin) ? "HIGH" : "LOW");
    printf("Estado del pin %d (LED): %s\n", led_pin, 
           digitalRead(led_pin) ? "HIGH" : "LOW");
    printf("✓ Test 2 completado\n\n");
    
    // Test 3: blink function con diferentes frecuencias
    printf("--- Test 3: Función blink ---\n");
    
    printf("Parpadeo lento: 1 Hz por 3 segundos\n");
    blink(led_pin, 1.0, 3.0);
    sleep(1);
    
    printf("Parpadeo medio: 3 Hz por 3 segundos\n");
    blink(led_pin, 3.0, 3.0);
    sleep(1);
    
    printf("Parpadeo rápido: 8 Hz por 2 segundos\n");
    blink(led_pin, 8.0, 2.0);
    sleep(1);
    
    printf("✓ Test 3 completado\n\n");
    
    // Test 4: Secuencia de parpadeos con patrón
    printf("--- Test 4: Patrón de parpadeos ---\n");
    printf("Ejecutando patrón: SOS en código Morse\n");
    
    // S (3 parpadeos cortos)
    printf("S: ");
    for(int i = 0; i < 3; i++) {
        blink(led_pin, 5.0, 0.2);  // 5 Hz por 0.2 seg = 1 parpadeo rápido
        usleep(200000); // 0.2 seg de pausa
    }
    usleep(500000); // 0.5 seg de pausa entre letras
    
    // O (3 parpadeos largos)
    printf("O: ");
    for(int i = 0; i < 3; i++) {
        blink(led_pin, 1.67, 0.6);  // 1.67 Hz por 0.6 seg = 1 parpadeo lento
        usleep(200000); // 0.2 seg de pausa
    }
    usleep(500000); // 0.5 seg de pausa entre letras
    
    // S (3 parpadeos cortos)
    printf("S: ");
    for(int i = 0; i < 3; i++) {
        blink(led_pin, 5.0, 0.2);  // 5 Hz por 0.2 seg = 1 parpadeo rápido
        usleep(200000); // 0.2 seg de pausa
    }
    
    printf("✓ Test 4 completado (SOS enviado)\n\n");
    
    // Test 5: Verificación final
    printf("--- Test 5: Verificación final ---\n");
    printf("Encendiendo LED para verificación visual...\n");
    digitalWrite(led_pin, HIGH);
    sleep(2);
    printf("Apagando LED...\n");
    digitalWrite(led_pin, LOW);
    printf("✓ Test 5 completado\n\n");
    
    // Mostrar información de los pines al final
    printf("--- Estado final de pines ---\n");
    printf("GPIO %d (LED): %s\n", led_pin, digitalRead(led_pin) ? "HIGH" : "LOW");
    printf("GPIO %d (BOTÓN): %s\n", button_pin, digitalRead(button_pin) ? "HIGH" : "LOW");
    printf("\n");
    
    // Limpiar recursos
    gpioCleanup();
    
    printf("========================================\n");
    printf("     Todos los tests completados!\n");
    printf("   Biblioteca Pi 4 funcionando OK\n");
    printf("========================================\n");
    
    return 0;
}