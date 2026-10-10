#include "board_fixtures.hpp"

#include "board/board.hpp"
#include <doctest/doctest.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>
#include <stdexcept>

TEST_CASE("Constructor por defecto") {
    naidu::Board b;

    fixtures::check_default(b, 3);
}

TEST_CASE("Constructor por defecto con dimesion arbitraria") {
    std::size_t dim = 7;
    naidu::Board b(dim);

    fixtures::check_default(b, dim);
}

TEST_CASE("Constructor desde vector con casillas correctas") {
    std::vector<std::uint8_t> tiles = fixtures::solved_4x4();
    naidu::Board b(tiles);

    CHECK(b.tile_count() == tiles.size());
    CHECK(b.dim() == static_cast<std::size_t>(std::sqrt(tiles.size())));

    for (std::size_t i = 0; i < tiles.size(); ++i)
        CHECK(b.at(i) == tiles[i]);
}

TEST_CASE("Constructor desde vector con casilla fuera de rango") {
    std::vector<std::uint8_t> tiles = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 99,
        13, 14, 15,  0
    };
    CHECK_THROWS_AS(naidu::Board{tiles}, std::invalid_argument);
}

TEST_CASE("Constructor desde vector con casillas duplicadas") {
    std::vector<std::uint8_t> tiles = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 11,
        13, 14, 15,  0
    };
    CHECK_THROWS_AS(naidu::Board{tiles}, std::invalid_argument);
}

TEST_CASE("Constructor desde vector con sin casilla vacia") {
    std::vector<std::uint8_t> tiles = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16
    };
    CHECK_THROWS_AS(naidu::Board{tiles}, std::invalid_argument);
}