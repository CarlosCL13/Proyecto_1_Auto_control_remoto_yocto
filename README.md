# Proyecto_1_Auto_control_remoto_yocto
Sistema embebido en Raspberry Pi 4 con Linux mínimo generado con Yocto. Permite controlar un vehículo remoto mediante servidor web: movimiento (PWM), luces, transmisión de video en tiempo real y librería de control propia. Incluye desarrollo cruzado y soporte para sensores opcionales.

# 🚗 Remote Vehicle Control with Yocto

Este proyecto implementa un **sistema embebido a la medida** para el control remoto de un vehículo autónomo mediante un **servidor web**.  
El sistema está basado en una **Raspberry Pi 4** con una imagen mínima de Linux creada con **Yocto Project**, optimizada para recursos limitados.

## 📌 Características principales
- **Control de movimiento**: Adelante, atrás, izquierda y derecha, con control de velocidad mediante PWM.  
- **Luces indicadoras**: LEDs delanteros, traseros y direccionales (control automático y manual).  
- **Transmisión de video en tiempo real** desde cámara conectada al sistema embebido.  
- **Biblioteca propia** para control de GPIO, motores, LEDs y cámara.  
- **Servidor web** para panel de control accesible vía WiFi/Bluetooth.  
- **Desarrollo cruzado**: compilación en host, ejecución en target.  

### Opcionales:
- Detección de obstáculos con sensores ultrasónicos/infrarrojos.  
- Captura de fotografías durante la operación.  

---

## 🛠️ Tecnologías y herramientas
- **Yocto Project** – construcción de imagen mínima de Linux.  
- **CMake/Autotools** – sistema de construcción de software.  
- **C/C++** – librerías para control de hardware.  
- **Servidor web embebido** – interfaz de usuario para control.  
- **Git/GitHub Flow** – gestión del repositorio.  

---

## ⚙️ Instalación y compilación

1. **Clonar el repositorio**  
   ```bash
   git clone https://github.com/<usuario>/Proyecto_1_Auto_control_remoto_yocto.git
   cd Proyecto_1_Auto_control_remoto_yocto

