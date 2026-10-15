# Guía de Instalación y Configuración Multiplataforma

## 1. Dependencias Generales del Proyecto

Independientemente del sistema operativo, el proyecto requiere estrictamente:

* **Compilador C++20**: Soporte completo para características modernas como `std::span` (GCC 11+, Clang 13+ o MSVC 2019+).
* **CMake**: Versión 3.16 o superior.
* **Qt6**: Módulos requeridos: `Core`, `Gui`, `Widgets`, `Concurrent` y `Charts`.

---

## 2. Instalación en Linux (Distribuciones Basadas en Debian / Ubuntu)

Abra la terminal y ejecute los siguientes comandos para instalar el compilador, CMake y las librerías de desarrollo de Qt6:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev libqt6charts6-dev
```

---

## 3. Instalación en Arch Linux

Arch Linux incluye herramientas y versiones de Qt muy actualizadas a través de su gestor de paquetes oficial `pacman`. Ejecute el siguiente comando para preparar su entorno:

```bash
sudo pacman -Syu
sudo pacman -S base-devel cmake qt6-base qt6-charts
```

---

## 4. Instalación en Windows

Para compilar y ejecutar la aplicación en entornos Windows, se recomienda el uso de **Visual Studio** o el entorno **MinGW-w64** junto con el instalador oficial de Qt.

### Opción A: Utilizando el Instalador Oficial de Qt (Recomendado)

1. Descargue e instale el **Qt Online Installer** desde el sitio web oficial de Qt.
2. Durante el proceso de instalación, seleccione la versión más reciente de **Qt 6** junto con el kit de compilación correspondiente (por ejemplo, **MSVC 2022 64-bit** o **MinGW**).
3. Asegúrese de marcar el módulo **Qt Charts** dentro de los componentes opcionales de Qt.
4. Instale **CMake** desde su sitio web oficial y agréguelo a las variables de entorno del sistema (`PATH`).

### Opción B: Utilizando MSYS2 (Para usuarios de MinGW)

Abra la terminal de MSYS2 UCRT64 e instale las dependencias con el siguiente comando:

```bash
pacman -S mingw-w64-ucrt64-toolchain mingw-w64-ucrt64-cmake mingw-w64-ucrt64-qt6-base mingw-w64-ucrt64-qt6-charts
```

---

## 5. Proceso de Compilación (Común para Todos los Sistemas)

Una vez instaladas las herramientas y dependencias correspondientes a su sistema operativo, abra una terminal en el directorio raíz del proyecto y siga estos pasos:

1. **Crear y acceder al directorio de construcción:**

   ```bash
   mkdir build
   ```

2. **Configurar el proyecto con CMake:**
   * *En Linux / Arch / MSYS2:*

     ```bash
     cmake -B bulid -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
     ```

   * *En Windows (Visual Studio / MSVC nativo):*

     ```cmd
     cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
     ```

3. **Compilar el código fuente:**
   * *Linux / macOS / MSYS2:*

     ```bash
     cmake --build build -j$(nproc)

     ```

   * *Windows (MSVC):*

     ```cmd
     cmake --build build --config Release
     ```

4. **Ejecutar la aplicación:**
   El ejecutable se ubicará dentro de la carpeta `build/bin` (o `build/Release/` en Windows bajo MSVC).
