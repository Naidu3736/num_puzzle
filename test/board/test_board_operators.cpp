#include "board_fixtures.hpp"

#include "board/board.hpp"
#include <doctest/doctest.h>

TEST_CASE("Operators: operator== tableros iguales") {
    naidu::Board a(4);
    naidu::Board b(4);
    CHECK(a == b);
}

TEST_CASE("Operators: operator== tableros distintos") {
    naidu::Board a(4);
    naidu::Board b(4);
    b.move(naidu::Direction::UP);
    CHECK_FALSE(a == b);
}

TEST_CASE("Operators: operator== dimensiones distintas") {
    naidu::Board a(3);
    naidu::Board b(4);
    CHECK_FALSE(a == b);
}