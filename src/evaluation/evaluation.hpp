#pragma once

#include "board/board.hpp"

#include <cstdint>

namespace naidu::evaluation {

    /**
     * @brief Firma común de las funciones de evaluación.
     *
     * Todas las funciones de evaluación (baseline, evaluation_1/2/3) comparten
     * esta firma para que el solver pueda seleccionarlas en tiempo de ejecución.
     *
     * Una función de evaluación calcula f(n) = g(n) + h(n):
     *   - b:        estado actual.
     *   - goal_pos: mapa de posiciones meta (goal_pos[ficha] = índice meta).
     *   - g:        costo acumulado desde el inicio hasta b.
     */
    using EvaluationFn = std::uint16_t(*)(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

    /**
     * @brief Evaluación 1: Manhattan + fichas mal colocadas + dispersión.
     *
     * Combina tres componentes:
     *
     *   f(n) = g(n) + manhattan(n) + misplaced(n) + w(dim) · dispersion(n)
     *
     * donde:
     *   - manhattan(n)    suma de distancias Manhattan de cada ficha a su meta.
     *                     ← component::manhattan_total(b, goal_pos)
     *   - misplaced(n)    fichas fuera de su meta.
     *                     ← component::misplaced_tiles(b, goal_pos)
     *   - dispersion(n)   distancia Manhattan media del hueco a cada ficha mal colocada.
     *                     ← component::blank_dispersion(b, goal_pos)
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
     * @brief Evaluación 2: Manhattan + fichas mal colocadas + proximidad.
     *
     * Combina tres componentes:
     *
     *   f(n) = g(n) + manhattan(n) + misplaced(n) - near(n) / 2
     *
     * donde:
     *   - manhattan(n)    suma de distancias Manhattan de cada ficha a su meta.
     *                     ← component::manhattan_total(b, goal_pos)
     *   - misplaced(n)    fichas fuera de su meta.
     *                     ← component::misplaced_tiles(b, goal_pos)
     *   - near(n)         fichas mal colocadas a Manhattan 1 de su meta.
     *                     ← component::tiles_at_distance_1(b, goal_pos)
     *
     * El término -near(n) introduce una recompensa por fichas casi
     * resueltas, sesgando la búsqueda hacia configuraciones próximas a la meta.
     *
     * @param b Tablero actual.
     * @param goal_pos Mapa de posiciones meta (positions[ficha] = índice meta).
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f para el estado dado.
     */
    std::uint16_t evaluation_2(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

    /**
     * @brief Evaluación 3: estructura local.
     *
     * Combina dos componentes:
     *
     *   f(n) = g(n) + manhattan(n) + broken_pairs(n)
     *
     * donde:
     *   - manhattan(n)     suma de distancias Manhattan de cada ficha a su meta.
     *                      ← component::manhattan_total(b, goal_pos)
     *   - broken_pairs(n)  pares adyacentes mal ordenados respecto a la meta.
     *                      ← component::broken_pairs(b, goal_pos)
     *
     * Penaliza el desorden estructural (pares adyacentes mal ordenados)
     * que Manhattan no captura. Un estado con muchas adyacencias bien
     * ordenadas se considera más prometedor que uno con estructura local
     * rota, aunque la distancia Manhattan sea similar.
     *
     * @param b Tablero actual.
     * @param goal_pos Mapa de posiciones meta (positions[ficha] = índice meta).
     * @param g Costo acumulado desde el estado inicial.
     * @return Valor de f para el estado dado.
     */
    std::uint16_t evaluation_3(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept;

}