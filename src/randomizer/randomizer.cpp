#include "randomizer.hpp"
#include "board/board.hpp"
#include "board/board_hash.hpp"
#include "core/direction.hpp"
#include "core/moveset.hpp"

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <random>

namespace  {
    std::uint32_t resolve_moves(std::uint8_t dim, std::uint32_t moves) {
        if (moves != 0) 
            return moves;

        return static_cast<std::uint32_t>(dim * dim * 20);
    }

    std::uint32_t resolve_max_attempts(std::uint32_t max_attempts, std::uint32_t moves) {
        if (max_attempts != 0)
            return max_attempts;

        return moves * 10;
    }

    naidu::Direction rand_direction(
        const naidu::MoveSet& legal_moves, 
        const std::optional<naidu::Direction> prev,
        std::mt19937& rng
    ) {
        std::uniform_int_distribution<std::uint8_t> dist(0, 3);

        while (true) {
            naidu::Direction d = naidu::ALL_DIRECTIONS[dist(rng)];

            if (prev && *prev == naidu::opposite(d))
                continue;

            if (legal_moves.contains(d))
                return d;
        }
    }

    void random_walk(
        naidu::Board& board,
        std::uint32_t moves,
        std::mt19937& rng
    ) {
        std::optional<naidu::Direction> prev;

        for (std::uint32_t i = 0; i < moves; ++i) {
            naidu::Direction d = rand_direction(board.legal_moves(), prev, rng);

            if (board.move(d))
                prev = d;
        }
    }

    void random_walk_unique(
        naidu::Board& board,
        std::uint32_t moves,
        std::uint32_t max_attempts,
        std::mt19937& rng
    ) {
        std::unordered_set<naidu::Board, naidu::BoardHash> visited;
        visited.insert(board);

        std::optional<naidu::Direction> prev;

        std::uint32_t applied = 0;
        std::uint32_t attempts = 0;

        while (applied < moves && attempts < max_attempts) {
            ++attempts;
            naidu::Direction d = rand_direction(board.legal_moves(), prev, rng);
            
            if (!board.move(d))
                continue;

            const auto [it, inserted] = visited.insert(board);
            if (!inserted) {
                board.move(naidu::opposite(d));
                continue;
            }

            ++applied;
            prev = d;
        }
    }

}

namespace naidu::randomizer {
    
    Board generate(std::uint8_t dim, const Config& config, std::mt19937& rng) {
        Board board(dim);

        const auto moves = resolve_moves(dim, config.moves);
        const auto max_attempts = resolve_max_attempts(
            config.max_attempts, moves
        );

        if (!config.avoid_revisits) {
            random_walk(board, moves, rng);
        } else {
            random_walk_unique(board, moves, max_attempts, rng);
        }

        return board;
    }

    Board generate(std::uint8_t dim, std::mt19937& rng) {
        return generate(dim, Config{}, rng);
    }

}