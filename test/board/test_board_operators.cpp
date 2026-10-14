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

TEST_CASE("Operators: operator!= tableros iguales") {
    naidu::Board a(4);
    naidu::Board b(4);
    CHECK_FALSE(a != b);
}

TEST_CASE("Operators: operator!= tableros distintos") {
    naidu::Board a(4);
    naidu::Board b(4);
    b.move(naidu::Direction::UP);
    CHECK(a != b);
}

TEST_CASE("Operators: operator!= dimensiones distintas") {
    naidu::Board a(3);
    naidu::Board b(4);
    CHECK(a != b);
}

TEST_CASE("Operators: operator!= es la negacion de operator==") {
    naidu::Board a({
        1, 2, 3,
        4, 5, 6,
        7, 8, 0,
    });
    naidu::Board b({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });

    CHECK(!(a == b) == (a != b));
}