#include "board/board.hpp"
#include "board/board_hash.hpp"
#include "core/direction.hpp"

#include <doctest/doctest.h>
#include <unordered_set>

TEST_CASE("US: Colision") {
    std::unordered_set<naidu::Board, naidu::BoardHash> visited;

    naidu::Board a(3);
    visited.insert(a);
    naidu::Board b(3);
    const auto [it, inserted] = visited.insert(b);

    CHECK(!inserted);
}

TEST_CASE("US: Sin colision") {
    std::unordered_set<naidu::Board, naidu::BoardHash> visited;

    naidu::Board a(3);
    visited.insert(a);
    a.move(naidu::Direction::UP);
    const auto [it, inserted] = visited.insert(a);

    CHECK(inserted);
}