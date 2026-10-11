#pragma once

#include "board/board.hpp"

#include <cstdint>
#include <span>

namespace naidu::component {

    // --------------------------------------------------------
    // Componente de costo
    // --------------------------------------------------------

    /**
     * @brief Costo de transición entre dos estados adyacentes.
     * @param previous Estado anterior.
     * @param current Estado actual (tras un movimiento).
     * @param goal_pos Mapa de posiciones meta (positions[ficha] = índice meta).
     * @return Costo del movimiento (1 o 2).
     */
    std::uint16_t step_cost(
        const Board& previous,
        const Board& current,
        std::span<const std::uint8_t> goal_pos
    ) noexcept;

    // --------------------------------------------------------
    // Componentes heurísticos
    // --------------------------------------------------------

    /**
     * @brief Suma de distancias Manhattan de todas las fichas a su meta.
     * @param b Tablero a evaluar.
     * @param goal_pos Mapa de posiciones meta.
     * @return Heurística de Manhattan.
     */
    std::uint16_t manhattan_total(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    );

    /**
     * @brief Número de pares adyacentes mal ordenados (desorden local).
     * @param b Tablero a evaluar.
     * @param goal_pos Mapa de posiciones meta.
     * @return Conteo de pares rotos.
     */
    std::uint16_t broken_pairs(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    );

    /**
     * @brief Número de fichas fuera de su meta.
     * @param b Tablero a evaluar.
     * @param goal_pos Mapa de posiciones meta.
     * @return Conteo de fichas mal colocadas.
     */
    std::uint16_t misplaced_tiles(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    );

    /**
     * @brief Fichas mal colocadas a Manhattan 1 de su meta.
     * @param b Tablero a evaluar.
     * @param goal_pos Mapa de posiciones meta.
     * @return Número de fichas "casi resueltas".
     */
    std::uint16_t tiles_at_distance_1(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    );

    /**
     * @brief Dispersión promedio del hueco respecto a fichas mal colocadas.
     *
     * Métrica de desorden: distancia Manhattan media entre el hueco y
     * cada ficha fuera de su meta. Se devuelve con precisión completa;
     * cada función de evaluación decide cómo tratarla.
     *
     * @param b Tablero a evaluar.
     * @param goal_pos Mapa de posiciones meta.
     * @return Distancia media en celdas. 0.0 si el tablero está resuelto.
     */
    double blank_dispersion(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    );

}