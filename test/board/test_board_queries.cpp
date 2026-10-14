#include "board_fixtures.hpp"

#include "board/board.hpp"
#include <doctest/doctest.h>
#include <cstddef>
#include <cstdint>
#include <vector>

TEST_CASE("Queries: Consistencia entre at(index) y at(row, col)") {
    std::vector<std::uint8_t> tiles = fixtures::unsolvable_4x4();
    naidu::Board b(tiles);

    std::size_t i = 0;
    for (std::size_t r = 0; r < b.dim(); ++r)
        for (std::size_t c = 0; c < b.dim(); ++c)
            CHECK(b.at(i++) == b.at(r, c));
}

TEST_CASE("Queries: Consistencia entre tiles() y at(index)") {
    naidu::Board b(fixtures::unsolvable_4x4());

    for (std::size_t i = 0; i < b.tile_count(); ++i)
        CHECK(b.tiles()[i] == b.at(i));
}

TEST_CASE("Queries: peek_empty coincide con el índice del vecino") {
    naidu::Board b({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });

    CHECK(b.peek_empty(naidu::Direction::UP)    == 4);
    CHECK(b.peek_empty(naidu::Direction::LEFT)  == 6);
    CHECK(b.peek_empty(naidu::Direction::RIGHT) == 8);
}

TEST_CASE("Queries: peek_tile devuelve la ficha del vecino") {
    naidu::Board b({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });

    CHECK(b.peek_tile(naidu::Direction::UP)    == 5);
    CHECK(b.peek_tile(naidu::Direction::LEFT)  == 7);
    CHECK(b.peek_tile(naidu::Direction::RIGHT) == 8);
}

TEST_CASE("Queries: peek_* no modifica el tablero") {
    naidu::Board b({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });
    const naidu::Board original = b;

    b.peek_empty(naidu::Direction::UP);
    b.peek_tile(naidu::Direction::UP);
    b.peek_empty(naidu::Direction::LEFT);
    b.peek_tile(naidu::Direction::RIGHT);

    CHECK(b == original);
}

TEST_CASE("Queries: Tablero resuelto") {
    naidu::Board b(4);
    CHECK(b.is_solved());
}

TEST_CASE("Queries: Tablero no resuelto") {
    naidu::Board b(fixtures::solvable_unsolved_4x4());
    CHECK_FALSE(b.is_solved());
}

TEST_CASE("Queries: Tablero par resoluble") {
    naidu::Board b(fixtures::solvable_unsolved_4x4());
    CHECK(b.is_solvable());
}

TEST_CASE("Queries: Tablero par no resoluble") {
    naidu::Board b(fixtures::unsolvable_4x4());
    CHECK_FALSE(b.is_solvable());
}

TEST_CASE("Queries: Tablero impar resoluble") {
    naidu::Board b(fixtures::solvable_5x5());
    CHECK(b.is_solvable());
}

TEST_CASE("Queries: Tablero impar no resoluble") {
    naidu::Board b(fixtures::unsolvable_5x5());
    CHECK_FALSE(b.is_solvable());
}

TEST_CASE("Queries: Tablero par resoluble con hueco en el centro") {
    naidu::Board b(fixtures::solvable_hole_center_4x4());
    CHECK(b.is_solvable());
}