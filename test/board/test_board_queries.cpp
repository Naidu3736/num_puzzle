#include "board_fixtures.hpp"

#include "board/board.hpp"
#include <doctest/doctest.h>
#include <cstddef>
#include <cstdint>
#include <vector>

TEST_CASE("Consistencia entre at(index) y at(row, col)") {
    std::vector<std::uint8_t> tiles = fixtures::unsolvable_4x4();
    naidu::Board b(tiles);

    std::size_t i = 0;
    for (std::size_t r = 0; r < b.dim(); ++r)
        for (std::size_t c = 0; c < b.dim(); ++c)
            CHECK(b.at(i++) == b.at(r, c));
}

TEST_CASE("Consistencia entre tiles() y at(index)") {
    naidu::Board b(fixtures::unsolvable_4x4());

    for (std::size_t i = 0; i < b.tile_count(); ++i)
        CHECK(b.tiles()[i] == b.at(i));
}

TEST_CASE("Tablero resuelto") {
    naidu::Board b(4);
    CHECK(b.is_solved());
}

TEST_CASE("Tablero no resuelto") {
    naidu::Board b(fixtures::solvable_unsolved_4x4());
    CHECK_FALSE(b.is_solved());
}   

TEST_CASE("Tablero par resoluble") {
    naidu::Board b(fixtures::solvable_unsolved_4x4());
    CHECK(b.is_solvable());
}

TEST_CASE("Tablero par no resoluble") {
    naidu::Board b(fixtures::unsolvable_4x4());
    CHECK_FALSE(b.is_solvable());
}

TEST_CASE("Tablero impar resoluble") {
    naidu::Board b(fixtures::solvable_5x5());
    CHECK(b.is_solvable());
}

TEST_CASE("Tablero impar no resoluble") {
    naidu::Board b(fixtures::unsolvable_5x5());
    CHECK_FALSE(b.is_solvable());
}

TEST_CASE("Tablero par resoluble con hueco en el centro") {
    naidu::Board b(fixtures::solvable_hole_center_4x4());
    CHECK(b.is_solvable());
}