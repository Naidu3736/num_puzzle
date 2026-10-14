#include "component.hpp"

#include <cmath>
#include <cstdint>
#include <span>

namespace {

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
        const Board& current,
        std::uint8_t old_empty_index,
        std::span<const std::uint8_t> goal_pos,
        bool weighted
    ) noexcept {
        if (!weighted)
            return 1;

        const auto new_empty = current.empty_index();
        const auto moved_tile = current.at(old_empty_index);

        if (moved_tile == 0)
            return 1;

        return (goal_pos[moved_tile] == new_empty) ? 2 : 1;
    }

    std::uint16_t manhattan_total(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    ) {
        std::uint16_t total = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            total += manhattan(i, goal_pos[tile], b.dim());
        }

        return total;
    }

    std::uint16_t broken_pairs(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    ) {
        const auto dim = b.dim();
        std::uint16_t broken = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;

            if ((i % dim) + 1 < dim) {
                const auto right = b.at(i + 1);
                if (right != 0
                    && !well_ordered_right(goal_pos[tile], goal_pos[right], dim))
                    ++broken;
            }

            if ((i / dim) + 1 < dim) {
                const auto below = b.at(i + dim);
                if (below != 0
                    && !well_ordered_below(goal_pos[tile], goal_pos[below], dim))
                    ++broken;
            }
        }

        return broken;
    }

    std::uint16_t misplaced_tiles(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    ) {
        std::uint16_t bad = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (goal_pos[tile] != i)
                ++bad;
        }

        return bad;
    }

    std::uint16_t tiles_at_distance_1(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    ) {
        std::uint16_t tiles = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (manhattan(i, goal_pos[tile], b.dim()) == 1)
                ++tiles;
        }

        return tiles;
    }

    double blank_dispersion(
        const Board& b,
        std::span<const std::uint8_t> goal_pos
    ) {
        std::uint16_t total_distance = 0;
        std::uint16_t misplaced = 0;

        for (std::uint8_t i = 0; i < b.tile_count(); ++i) {
            const auto tile = b.at(i);
            if (tile == 0)
                continue;
            if (goal_pos[tile] == i)
                continue;
            total_distance += manhattan(b.empty_index(), i, b.dim());
            ++misplaced;
        }

        return misplaced == 0
            ? 0.0
            : static_cast<double>(total_distance) / misplaced;
    }

}