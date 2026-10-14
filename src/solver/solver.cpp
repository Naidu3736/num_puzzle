#include "solver.hpp"
#include "board/board.hpp"
#include "board/board_hash.hpp"
#include "core/direction.hpp"
#include "evaluation/component.hpp"

#include <optional>
#include <unordered_set>
#include <cstdint>
#include <vector>

namespace {

    /**
     * @brief Contexto compartido por todas las llamadas recursivas de `search`.
     *
     * Agrupa la configuración de la búsqueda, el tablero meta, su mapa de
     * posiciones, el estado mutable (nodos expandidos, camino actual, visitados)
     * y la información de la solución cuando se encuentra.
     */
    struct Context {
        const naidu::Config& config;                                         ///< Configuración de la búsqueda.
        const naidu::Board& goal;                                            ///< Tablero meta.
        std::vector<std::uint8_t> goal_pos;                                  ///< positions[ficha] = índice meta.
        std::uint64_t nodes_expanded = 0;                                    ///< Contador de nodos expandidos.
        std::vector<naidu::Direction> path;                                  ///< Camino actual (direcciones).
        std::unordered_set<naidu::Board, naidu::BoardHash> visited;          ///< Estados del camino actual.
        std::uint16_t solution_cost = 0;                                     ///< Costo de la solución encontrada.
        bool found = false;                                                  ///< ¿Se encontró la solución?
    };

    /**
     * @brief Construye el mapa de posiciones meta.
     *
     * Para cada ficha, guarda el índice donde debería estar según `goal`.
     * El resultado se usa para calcular Manhattan, dispersión, etc., y para
     * evaluar el costo ponderado de los movimientos.
     *
     * @param goal Tablero meta.
     * @return Vector donde `positions[ficha] = índice meta`.
     */
    std::vector<std::uint8_t> build_goal_positions(const naidu::Board& goal) {
        std::vector<std::uint8_t> positions(goal.tile_count());
        for (std::uint8_t i = 0; i < goal.tile_count(); ++i)
            positions[goal.at(i)] = i;
        return positions;
    }

    /**
     * @brief Intenta entrar en un nodo hijo moviendo el hueco en la dirección `d`.
     *
     * Modifica `node` y `ctx.path` como efecto secundario. Calcula el costo
     * del movimiento con `component::step_cost`. Si `avoid_revisits` está
     * activo y el hijo ya estaba en `ctx.visited`, deshace el movimiento,
     * restaura `ctx.path` y devuelve false.
     *
     * @param node Tablero actual (modificado in-place).
     * @param d Dirección del movimiento.
     * @param ctx Contexto compartido.
     * @param cost Costo del movimiento (salida, solo válido si devuelve true).
     * @return true si se entró correctamente; false si el hijo ya estaba visitado.
     */
    bool try_entry(
        naidu::Board& node,
        naidu::Direction d,
        Context& ctx,
        std::uint16_t& cost
    ) {
        const auto moved_tile = node.peek_tile(d);
        const auto new_empty_index = node.peek_empty(d);

        node.move(d);
        ctx.path.push_back(d);

        if (ctx.config.avoid_revisits) {
            const auto [it, inserted] = ctx.visited.insert(node);
            if (!inserted) {
                ctx.path.pop_back();
                node.move(naidu::opposite(d));

                return false;
            }
        }

        cost = naidu::component::step_cost(
            moved_tile,
            new_empty_index,
            ctx.goal_pos,
            ctx.config.weighted_cost
        );

        return true;
    }

    /**
     * @brief Deshace la entrada a un nodo hijo.
     *
     * Restaura el tablero, el camino y (si aplica) el conjunto de visitados.
     * Debe llamarse exactamente una vez por cada `try_entry` que devolvió true.
     *
     * @param node Tablero actual (modificado in-place).
     * @param d Dirección del movimiento realizado.
     * @param ctx Contexto compartido.
     */
    void leave(naidu::Board& node, naidu::Direction d, Context& ctx) {
        if (ctx.config.avoid_revisits)
            ctx.visited.erase(node);

        ctx.path.pop_back();
        node.move(naidu::opposite(d));
    }

    /**
     * @brief Búsqueda DFS con poda por umbral (núcleo de IDA*).
     *
     * @param node Tablero actual (modificado in-place, se restaura).
     * @param g Costo acumulado desde el inicio.
     * @param threshold Umbral actual (f máximo permitido).
     * @param ctx Contexto compartido.
     * @return std::nullopt si se encontró solución (ctx.found = true)
     *                    o si se superó el límite (ctx.found = false).
     *         El menor f que excedió el umbral, en caso contrario.
     */
    std::optional<std::uint32_t> search(
        naidu::Board& node,
        std::uint16_t g,
        std::uint32_t threshold,
        Context& ctx
    ) {
        ++ctx.nodes_expanded;

        if (ctx.config.max_nodes != 0 && ctx.nodes_expanded > ctx.config.max_nodes)
            return std::nullopt;

        auto f = ctx.config.eval_fn(node, ctx.goal_pos, g);
        if (f > threshold)
            return f;

        if (node == ctx.goal) {
            ctx.found = true;
            ctx.solution_cost = g;
            return std::nullopt;
        }

        std::optional<std::uint32_t> min;

        const auto moves = node.legal_moves();
        for (const auto d : naidu::ALL_DIRECTIONS) {
            if (!moves.contains(d))
                continue;

            std::uint16_t cost = 0;
            if (!try_entry(node, d, ctx, cost))
                continue;

            const std::uint16_t new_g = static_cast<std::uint16_t>(g + cost);
            const auto t = search(node, new_g, threshold, ctx);

            if (ctx.found)
                return std::nullopt;

            leave(node, d, ctx);

            if (t && (!min || *t < *min))
                min = t;
        }

        return min;
    }

}
namespace naidu {

    Solution solver(
        const Board& initial,
        const Board& goal,
        const Config& config
    ) {
        Context ctx{
            .config = config,
            .goal = goal,
            .goal_pos = build_goal_positions(goal),
            .nodes_expanded = 0,
            .path = {},
            .visited = {},
            .solution_cost = 0,
            .found = false
        };

        Board current = initial;

        std::uint32_t threshold = config.eval_fn(current, ctx.goal_pos, 0);

        while (true) {
            const auto t = search(current, 0, threshold, ctx);

            if (ctx.found) {
                return Solution{
                    .moves = ctx.path,
                    .cost = ctx.solution_cost,
                    .nodes_expanded = ctx.nodes_expanded,
                    .found = true
                };
            }

            if (!t){
                return Solution{
                    .moves = {},
                    .cost = 0,
                    .nodes_expanded = ctx.nodes_expanded,
                    .found = false
                };
            }

            threshold = *t;

            ctx.visited.clear();
        }
    }

}