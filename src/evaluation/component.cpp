#include "component.hpp"
#include "board/board.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace {

    using GoalPositions = std::array<std::uint8_t, naidu::Board::MAX_TILES>;

    GoalPositions goal_positions(
        const naidu::Board& board,
        const naidu::Board& goal
    ) {
        if (board.dim() != goal.dim())
            throw std::invalid_argument("Board and goal dimensions must match");

        GoalPositions positions{};
        for (std::uint8_t i = 0; i < goal.tile_count(); ++i)
            positions[goal.at(i)] = i;
        return positions;
    }

    std::uint16_t manhattan(
        std::uint8_t current_pos,
        std::uint8_t target_pos,
        std::uint8_t dim
    ) {
        const auto di = std::abs(
            static_cast<int>(current_pos / dim) - static_cast<int>(target_pos / dim));
        const auto dj = std::abs(
            static_cast<int>(current_pos % dim) - static_cast<int>(target_pos % dim));
        return static_cast<std::uint16_t>(di + dj);
    }

    bool well_ordered_right(
        std::uint8_t goal_here,
        std::uint8_t goal_right,
        std::uint8_t dim
    ) {
        return goal_here / dim == goal_right / dim
            && goal_here % dim + 1 == goal_right % dim;
    }

    bool well_ordered_below(
        std::uint8_t goal_here,
        std::uint8_t goal_below,
        std::uint8_t dim
    ) {
        return goal_here % dim == goal_below % dim
            && goal_here / dim + 1 == goal_below / dim;
    }

}

namespace naidu::component {

    std::uint16_t step_cost(
        const Board& previous,
        const Board& current
    ) noexcept {
        return 1;
    }

    std::uint16_t manhattan_total(const Board& b, const Board& goal) {
        const auto positions = goal_positions(b, goal);
        std::uint16_t total = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            total += manhattan(i, positions[tile], b.dim());
        }

        return total;
    }

    std::uint16_t broken_pairs(const Board& b, const Board& goal) {
        const auto positions = goal_positions(b, goal);
        const auto dim = b.dim();
        std::uint16_t broken = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;

            if ((i % dim) + 1 < dim) {
                const auto right = b.at(i + 1);
                if (right != 0
                    && !well_ordered_right(positions[tile], positions[right], dim))
                    ++broken;
            }

            if ((i / dim) + 1 < dim) {
                const auto below = b.at(i + dim);
                if (below != 0
                    && !well_ordered_below(positions[tile], positions[below], dim))
                    ++broken;
            }
        }

        return broken;
    }

    std::uint16_t misplaced_tiles(const Board& b, const Board& goal) {
        const auto positions = goal_positions(b, goal);
        std::uint16_t bad = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (positions[tile] != i)
                ++bad;
        }

        return bad;
    }

    std::uint16_t tiles_at_distance_1(const Board& b, const Board& goal) {
        const auto positions = goal_positions(b, goal);
        std::uint16_t tiles = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (manhattan(i, positions[tile], b.dim()) == 1)
                ++tiles;
        }

        return tiles;
    }

    double blank_dispersion(const Board& b, const Board& goal) {
        const auto positions = goal_positions(b, goal);
        std::uint16_t total_distance = 0;
        std::uint16_t misplaced = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (positions[tile] == i)
                continue;
            total_distance += manhattan(b.empty_index(), i, b.dim());
            ++misplaced;
        }

        return misplaced == 0
            ? 0.0
            : static_cast<double>(total_distance) / misplaced;
    }

}