cmake_minimum_required(VERSION 3.16)

project(num_puzzle VERSION 0.1 LANGUAGES CXX)

# ============================================================
# Opciones globales
# ============================================================

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# ============================================================
# Directorios de salida
# ============================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================
# IntelliSense / IDE
# ============================================================

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Dependencias
# ============================================================

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ============================================================
# Subdirectorios
# ============================================================

add_subdirectory(src)
add_subdirectory(test)