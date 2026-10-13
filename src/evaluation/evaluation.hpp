#pragma once

#include "board/board.hpp"

#include <cstdint>

namespace naidu::evaluation {

    /**
     * @brief Función de evaluación de referencia (baseline).
     *
     * f(n) = g(n) + manhattan(n) + (#fichas_correctas)
     *
     * Combina Manhattan con una recompensa por fichas bien colocadas.
     *
     * @param b Tablero actual.
     * @param goal_pos Mapa de posiciones meta.
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f para el estado dado.
     */
    std::uint16_t evaluation_baseline(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

    /**
     * @brief Evaluación 1: Manhattan + fichas mal colocadas + dispersión del hueco.
     *
     * Combina tres componentes:
     *
     *   f(n) = g(n) + manhattan(n) + misplaced(n) + w(dim) · dispersion(n)
     *
     * donde:
     *   - manhattan(n)    suma de distancias Manhattan de cada ficha a su meta.
     *   - misplaced(n)    fichas fuera de su meta.
     *   - dispersion(n)   distancia Manhattan media del hueco a cada ficha mal colocada.
     *   - w(dim)          peso adaptativo según el tamaño del tablero:
     *                       0.0   si dim ≤ 7   (dispersión ignorada)
     *                       0.5   si 8 ≤ dim ≤ 10
     *                       1.0   si dim > 10
     *
     * El peso crece con la dimensión porque en tableros grandes el tránsito
     * del hueco entre operaciones domina el costo total, y la dispersión
     * captura precisamente ese tránsito. En tableros pequeños la dispersión
     * no correlaciona con la dificultad y se ignora.
     *
     * @param b Tablero actual.
     * @param goal_pos Mapa de posiciones meta (positions[ficha] = índice meta).
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f para el estado dado.
     */
    std::uint16_t evaluation_1(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

    /**
     * @brief 
     * @param b Tablero actual.
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f₂.
     */
    std::uint16_t evaluation_2(
        const Board& b, 
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

    /**
     * @brief 
     * @param b Tablero actual.
     * @param g Costo acumulado (puede ser ponderado).
     * @return Valor de f₃.
     */
    std::uint16_t evaluation_3(
        const Board& b, 
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

}