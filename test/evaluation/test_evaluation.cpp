#include "board/board.hpp"
#include "evaluation/evaluation.hpp"

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

    double dispersion_weight(std::uint8_t dim) {
        if (dim > 10) return 1.0;
        if (dim > 7)  return 0.5;
        return 0.0;
    }

}

TEST_CASE("Evaluation: Evaluation 1 (Manhattan + misplaced + dispersion ponderada)") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);

        const auto g = std::uint16_t{0};
        const auto f = naidu::evaluation::evaluation_1(b, goal_pos, g);

        auto h = c.manhattan + c.bad_tiles;
        const auto w = dispersion_weight(b.dim());
        h += static_cast<std::uint16_t>(w * c.blank_dispersion);

        const auto expected = static_cast<std::uint16_t>(g + h);

        CHECK(f == expected);
    }
}

TEST_CASE("Evaluation: Evaluation 2 (Manhattan + misplaced + proximidad)") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);

        const auto g = std::uint16_t{0};
        const auto f = naidu::evaluation::evaluation_2(b, goal_pos, g);

        auto h = static_cast<int>(c.manhattan) + static_cast<int>(c.bad_tiles);
        h -= static_cast<int>(c.tiles_at_1_distance);

        const auto expected = static_cast<std::uint16_t>(g + h);

        CHECK(f == expected);
    }
}

TEST_CASE("Evaluation: Evaluation 3 (Manhattan + misplaced + broken pairs)") {
    for (const auto& [name, c] : fixtures::random_cases) {
        CAPTURE(name);
        naidu::Board b(c.tiles);
        naidu::Board goal(b.dim());
        const auto goal_pos = build_goal_positions(goal);

        const auto g = std::uint16_t{0};
        const auto f = naidu::evaluation::evaluation_3(b, goal_pos, g);

        auto h = static_cast<int>(c.manhattan);
        h += static_cast<int>(c.broken_pairs);

        const auto expected = static_cast<std::uint16_t>(g + h);

        CHECK(f == expected);
    }
}