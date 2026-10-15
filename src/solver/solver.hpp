#pragma once

#include "board/board.hpp"
#include "core/direction.hpp"
#include "evaluation/evaluation.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace naidu::solver {

    /**
     * @brief Resultado de la búsqueda.
     */
    struct Solution {
        std::vector<Direction> moves;   ///< Secuencia de movimientos (vacía si no se encontró).
        std::uint16_t cost;             ///< Costo total del camino.
        std::uint64_t nodes_expanded;   ///< Nodos expandidos durante la búsqueda.
        bool found;                     ///< ¿Se encontró solución?
    };

    /**
     * @brief Configuración de la búsqueda.
     */
    struct Config {
        evaluation::EvaluationFn eval_fn;                   ///< Función de evaluación
        std::function<bool()> cancel_requested = nullptr;   ///< Función para detener busqueda
        std::uint64_t max_nodes = 10'000'000;               ///< Límite de nodos expandidos.
        bool avoid_revisits = false;                        ///< Evitar revisitar estados en el camino actual
        bool weighted_cost = false;                         ///< Coste ponderado (true), uniforme (false)
    };
    
    /**
     * @brief Resuelve un num-puzzle usando IDA* (Iterative Deepening A*).
     *
     * Aplica el algoritmo IDA* con umbrales crecientes sobre la función
     * de evaluación f(n) = g(n) + h(n). Garantiza encontrar solución
     * óptima en costo si la evaluación es admisible; en caso contrario
     * devuelve una solución subóptima.
     *
     * @param initial Tablero inicial.
     * @param goal Tablero meta.
     * @param config Límites de nodos y profundidad.
     * @return Solución con la secuencia de movimientos, costo, nodos expandidos y flag found.
     */
    Solution ida(
        const Board& initial,
        const Board& goal,
        const Config& config = {}
    );
}