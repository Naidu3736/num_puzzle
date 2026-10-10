#pragma once

#include <cstdint>

namespace naidu {

    /**
     * @brief Direcciones de movimiento del hueco en el tablero.
     *
     * Los valores están asignados de forma que cada dirección y su opuesta
     * difieren en el bit menos significativo (ver opposite()).
     *
     *   UP  = 0  (0b00)   ↔   DOWN  = 1  (0b01)
     *   LEFT= 2  (0b10)   ↔   RIGHT = 3  (0b11)
     */
    enum class Direction : std::uint8_t {
        UP    = 0,   ///< Desplazamiento hacia arriba (fila - 1).
        DOWN  = 1,   ///< Desplazamiento hacia abajo (fila + 1).
        LEFT  = 2,   ///< Desplazamiento hacia la izquierda (columna - 1).
        RIGHT = 3,   ///< Desplazamiento hacia la derecha (columna + 1).
    };

    /** Número total de direcciones posibles. */
    inline constexpr std::uint8_t direction_count = 4;

    /**
     * @brief Convierte una dirección a su índice numérico subyacente.
     * @param d Dirección a convertir.
     * @return Valor entero en el rango [0, 3].
     */
    constexpr std::uint8_t to_index(Direction d) {
        return static_cast<std::uint8_t>(d);
    }

    /**
     * @brief Devuelve la dirección opuesta a la dada.
     *
     * Se calcula invirtiendo el bit menos significativo del índice.
     * UP ↔ DOWN, LEFT ↔ RIGHT.
     *
     * @param d Dirección original.
     * @return Dirección opuesta.
     */
    constexpr Direction opposite(Direction d) noexcept {
        return static_cast<Direction>(to_index(d) ^ 1u);
    }
}