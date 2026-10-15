# Puzzle Numérico con IDA*

## 1. Descripción General del Proyecto

Este proyecto es una aplicación de escritorio desarrollada en **C++** utilizando **Qt6** y **CMake**, diseñada para resolver tableros de puzzle deslizante numérico (desde 3x3 hasta 15x15) mediante el algoritmo de búsqueda informada **IDA\*** (*Iterative Deepening A\***).

La arquitectura prioriza la modularidad, el rendimiento de ejecución, y la capacidad de realizar análisis empíricos masivos mediante procesamiento asincrónico con `QtConcurrent` y visualización avanzada de datos con `QtCharts`.

---

## 2. Características Principales

- **Algoritmo IDA\*** optimizado en el espacio de nombres `naidu::solver`, con soporte para funciones heurísticas configurables, ponderación de costos y prevención de visitas redundantes.
- **Uso estricto de vistas de memoria (`std::span`)** para el manejo eficiente y sin copias innecesarias de arreglos de fichas y secuencias de estados.
- **Interfaz Gráfica (Qt6)** dividida en pestañas independientes (`PuzzleTab` para visualización y animación de soluciones paso a paso, y `AnalysisTab` para la experimentación por lotes de dificultad).
- **Ejecución Asíncrona:** Integración con `QFutureWatcher` y `QtConcurrent` para mantener la interfaz completamente fluida y responsiva durante cálculos masivos o prolongados, incluyendo lógica de cancelación de tareas por el usuario.
- **Suite de Análisis y Gráficas (`QtCharts`):** Generación de gráficas dinámicas de tiempo de ejecución por instancia, porcentajes de éxito frente a fallo y comparativas de costo real frente a movimientos de mezcla.
- **Exportación de Datos:** Funcionalidad para guardar capturas de todas las gráficas analíticas directamente en formato de imagen desde el menú principal de la aplicación (`MainWindow`).

---

## 3. Instalación y Configuración

Para instrucciones detalladas sobre cómo instalar las dependencias, configurar el entorno y compilar el proyecto en **Linux**, **Arch Linux** y **Windows**, consulte la [Guía de Instalación y Compilación](./installation_guiede.md).

---

## 4. Estructura del Código y Componentes Clave

- **`naidu::Board` (`board/`):** Representa el estado del tablero numérico de dimensión $N \times N$, haciendo uso intensivo de `std::span` para la validación y manipulación eficiente de fichas.
- **`naidu::solver` (`solver/`):** Contiene la implementación del algoritmo IDA* y las estructuras de configuración y control de cancelación de hilos.
- **`naidu::randomizer` (`randomizer/`):** Generador de instancias aleatorias basadas en movimientos válidos de mezcla.
- **`PuzzleTab` (`ui/puzzle/`):** Controla la interacción visual directa, la validación de entradas de texto o autogeneración de tableros meta estándar, y la animación por temporizadores de la solución obtenida.
- **`AnalysisTab` (`ui/analysis/`):** Administra la ejecución masiva de lotes de pruebas (Fácil, Medio, Difícil) mediante mapeo concurrente y actualiza las vistas de `QtCharts` evitando duplicidades en los ejes.

## 4. Documentación Técnica

La documentación completa del código fuente del motor de búsqueda y las estructuras de datos (excluyendo la interfaz gráfica) ha sido generada mediante Doxygen. Puede consultarla clonando el repositorio y abriendo el siguiente archivo en su navegador web:
- [Ver Documentación Técnica (HTML)](docs/html/index.html)
