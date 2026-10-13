#include "evaluation.hpp"
#include "component.hpp"

namespace naidu::evaluation {

    std::uint16_t evaluation_baseline(
        const Board& b, 
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept {
        auto h = component::manhattan_total(b, goal_pos);
        h -= (b.tile_count() - 1) - component::misplaced_tiles(b, goal_pos);

        return g + h; 
    }

    std::uint16_t evaluation_1(
        const Board& b, 
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept {
        auto h = component::manhattan_total(b, goal_pos);
        h += component::misplaced_tiles(b, goal_pos);


        if (b.dim() > 10) 
            h += static_cast<std::uint16_t>(component::blank_dispersion(b, goal_pos));
        else if (b.dim() > 7)
            h += static_cast<std::uint16_t>(component::blank_dispersion(b, goal_pos) / 2);

        return g + h;
    }

    std::uint16_t evaluation_2(
        const Board& b,
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept {
        auto h = component::manhattan_total(b, goal_pos);
        h += component::misplaced_tiles(b, goal_pos);

        h -= component::tiles_at_distance_1(b, goal_pos);

        return g + h;
    }

    std::uint16_t evaluation_3(
        const Board& b, 
        std::span<const std::uint8_t> goal_pos,
        std::uint16_t g
    ) noexcept {
        auto h = component::manhattan_total(b, goal_pos);
        h += component::broken_pairs(b, goal_pos);

        return g + h;
    }

}