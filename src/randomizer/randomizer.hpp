#pragma once

#include "board/board.hpp"

#include <cstdint>
#include <random>


namespace naidu::randomizer {

    /**
     * @brief Parámetros del random walk.
     */
    struct Config {
        std::uint32_t moves = 0;         ///< Número de movimientos a aplicar (0 = dim² · 20).
        bool avoid_revisits = true;      ///< Si true, evita repetir estados en el walk.
        std::uint32_t max_attempts = 0;  ///< Límite de intentos totales (0 = moves · 10).
    };

    /**
     * @brief Genera un tablero aleatorio resoluble.
     *
     * Aplica un random walk desde el estado resuelto. Garantiza que el
     * tablero resultante es soluble (los movimientos del hueco son
     * reversibles). Si `avoid_revisits` está activo, evita visitar el
     * mismo estado dos veces durante el walk.
     *
     * @param dim Dimensión del tablero (3..15).
     * @param config Parámetros del walk.
     * @param rng Generador de números aleatorios.
     * @return Tablero resoluble aleatorio.
     */
    Board generate(Board goal, const Config& config, std::mt19937& rng);

    /**
     * @brief Genera un tablero aleatorio con parámetros por defecto.
     * @param dim Dimensión del tablero (3..15).
     * @param rng Generador de números aleatorios.
     * @return Tablero resoluble aleatorio.
     */
    Board generate(Board goal, std::mt19937& rng);
}