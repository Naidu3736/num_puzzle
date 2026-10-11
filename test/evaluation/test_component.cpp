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

TEST_CASE("Distancia total de Manhattan") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::manhattan_total(b, goal_pos) == c.manhattan);
    }
}

TEST_CASE("Fichas fuera de su meta") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::misplaced_tiles(b, goal_pos) == c.bad_tiles);
    }
}

TEST_CASE("Fichas a 1 de distancia (Manhattan)") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::tiles_at_distance_1(b, goal_pos) == c.tiles_at_1_distance);
    }
}

TEST_CASE("Pares rotos") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::broken_pairs(b, goal_pos) == c.broken_pairs);
    }
}

TEST_CASE("Dispersion del hueco respecto a fichas mal colocadas") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);
        CHECK(naidu::component::blank_dispersion(b, goal_pos)
              == doctest::Approx(c.blank_dispersion));
    }
}