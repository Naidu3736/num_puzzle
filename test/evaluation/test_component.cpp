#include "board/board.hpp"
#include "evaluation/component.hpp"

#include "board_fixtures.hpp"

#include <doctest/doctest.h>

#include <cstdint>
#include <vector>

namespace {

    std::vector<std::uint8_t> build_goal_positions(const naidu::Board& goal) {
        std::vector<std::uint8_t> positions(goal.tile_count());
        for (std::uint8_t i = 0; i < goal.tile_count(); ++i)
            positions[goal.at(i)] = i;
        return positions;
    }

}

TEST_CASE("Component: step_cost no descoloca ficha (costo 1)") {
    naidu::Board current({
        1, 2, 3,
        4, 5, 6,
        7, 8, 0,
    });
    naidu::Board goal(3);
    const auto goal_pos = build_goal_positions(goal);

    const auto cost = naidu::component::step_cost(current, 7, goal_pos);
    CHECK(cost == 1);
}

TEST_CASE("Component: step_cost descoloca ficha correcta (costo 2)") {
    naidu::Board current({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });
    naidu::Board goal(3);
    const auto goal_pos = build_goal_positions(goal);

    const auto cost = naidu::component::step_cost(current, 8, goal_pos);
    CHECK(cost == 2);
}

TEST_CASE("Component: step_cost con meta arbitraria") {
    naidu::Board goal({
        1, 2, 3,
        4, 6, 5,
        7, 8, 0,
    });
    const auto goal_pos = build_goal_positions(goal);

    naidu::Board current({
        1, 2, 3,
        4, 0, 5,
        7, 6, 8,  
    });

    const auto cost = naidu::component::step_cost(current, 7, goal_pos);
    CHECK(cost == 2);
}

TEST_CASE("Component: step_cost devuelve 1 o 2 en todos los movimientos") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);

        const auto moves = b.legal_moves();
        for (const auto d : naidu::ALL_DIRECTIONS) {
            if (!moves.contains(d))
                continue;

            const auto old_empty = b.empty_index();

            naidu::Board next = b;
            next.move(d);

            const auto cost = naidu::component::step_cost(next, old_empty, goal_pos);
            CHECK((cost == 1 || cost == 2));
        }
    }
}

TEST_CASE("Component: Distancia total de Manhattan") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::manhattan_total(b, goal_pos) == c.manhattan);
    }
}

TEST_CASE("Component: Fichas fuera de su meta") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::misplaced_tiles(b, goal_pos) == c.bad_tiles);
    }
}

TEST_CASE("Component: Fichas a 1 de distancia (Manhattan)") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::tiles_at_distance_1(b, goal_pos) == c.tiles_at_1_distance);
    }
}

TEST_CASE("Component: Pares rotos") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::broken_pairs(b, goal_pos) == c.broken_pairs);
    }
}

TEST_CASE("Component: Dispersion del hueco respecto a fichas mal colocadas") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::blank_dispersion(b, goal_pos)
              == doctest::Approx(c.blank_dispersion));
    }
}