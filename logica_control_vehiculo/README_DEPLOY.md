# Guía de Compilación, Instalación y Despliegue

## 1. Compilación y generación de la biblioteca y ejecutable

### a) Preparar el entorno de cross-compilación
Asegúrate de tener el SDK y toolchain para la Raspberry Pi 4. Por ejemplo, usando Poky:

```bash
   rm -rf build/
   mkdir build && cd build && mkdir usr
. /opt/poky/5.0.11/environment-setup-cortexa7t2hf-neon-vfpv4-poky-linux-gnueabi
```

### b) Crear el directorio de build y configurar CMake
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake -DCMAKE_INSTALL_PREFIX=/home/ludwinr/Descargas/caloooos/AppWeb/build/usr
```

### c) Compilar e instalar
```bash
make
make install
```
Esto generará:
- El ejecutable en `build/usr/bin/server`
- La biblioteca compartida en `build/usr/lib/liblibgpio_rpi4.so`
- (Opcional) Headers en `build/usr/include/`

## 2. Archivos necesarios para ejecutar en la Raspberry Pi 4

### Debes transferir:
- `build/usr/bin/server` → El ejecutable principal.
- `build/usr/lib/liblibgpio_rpi4.so` → Biblioteca compartida necesaria para el ejecutable.
- Archivos web y de configuración: `index.html`, `control.html`, `control.js`, `auth.js`, `vehicleApi.js`, `styles.css`, `users.json`, etc.

### No necesitas transferir:
- Archivos fuente `.c` y `.h` (como `auth_utils.c`, `car_control.c`, etc.), a menos que vayas a compilar en la RPi4.
- Archivos de build temporales o de desarrollo.

### ¿Por qué?
- El ejecutable ya contiene el código compilado de los fuentes.
- La biblioteca `.so` es necesaria porque el ejecutable la usa dinámicamente.
- Los archivos web y de configuración son requeridos por la aplicación en tiempo de ejecución.
- Los archivos fuente solo son útiles para desarrollo o compilación, no para ejecución.

## 3. Comandos para transferir archivos

```bash
# Biblioteca compartida
scp /ruta/al/build/usr/lib/liblibgpio_rpi4.so root@192.168.100.139:/usr/lib/

# Ejecutable
scp /ruta/al/build/usr/bin/server root@192.168.100.139:/home/root/

# Archivos web y de configuración
scp /ruta/al/index.html /ruta/al/control.html /ruta/al/control.js /ruta/al/auth.js /ruta/al/vehicleApi.js /ruta/al/styles.css /ruta/al/users.json root@192.168.100.139:/home/root/
```

Ajusta las rutas según tu estructura.

---

**Resumen:**
Solo transfiere ejecutables, bibliotecas y archivos de recursos necesarios para la ejecución. Los fuentes solo se requieren para desarrollo o compilación en destino.