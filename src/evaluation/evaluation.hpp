#pragma once

#include "board/board.hpp"

#include <cstdint>

namespace naidu::evaluation {

    // --------------------------------------------------------
    // Funciones de evaluación
    // --------------------------------------------------------

    /**
     * @brief 
     * @param b Tablero actual.
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f₁.
     */
    std::uint16_t evaluation_1(const Board& b, std::uint16_t g) noexcept;

    /**
     * @brief 
     * Peso w adaptativo por tamaño del tablero.
     * @param b Tablero actual.
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f₂.
     */
    std::uint16_t evaluation_2(const Board& b, std::uint16_t g) noexcept;

    /**
     * @brief 
     * @param b Tablero actual.
     * @param g Costo acumulado (puede ser ponderado).
     * @return Valor de f₃.
     */
    std::uint16_t evaluation_3(const Board& b, std::uint16_t g) noexcept;

}