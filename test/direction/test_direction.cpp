#include <initializer_list>
#include <doctest/doctest.h>

#include "core/direction.hpp"

using naidu::Direction;
using naidu::to_index;
using naidu::opposite;
using naidu::direction_count;

TEST_CASE("Direction: to_index mapea a 0..3") {
    CHECK(to_index(Direction::UP)    == 0);
    CHECK(to_index(Direction::DOWN)  == 1);
    CHECK(to_index(Direction::LEFT)  == 2);
    CHECK(to_index(Direction::RIGHT) == 3);
}

TEST_CASE("Direction: los índices son únicos y cubren 0..3") {
    bool seen[4] = {false, false, false, false};
    for (Direction d : {Direction::UP, Direction::DOWN,
                        Direction::LEFT, Direction::RIGHT}) {
        const auto i = to_index(d);
        REQUIRE(i < direction_count);
        CHECK_FALSE(seen[i]);
        seen[i] = true;
    }
    for (bool s : seen) CHECK(s);
}

TEST_CASE("Direction: opposite invierte vertical y horizontalmente") {
    CHECK(opposite(Direction::UP)    == Direction::DOWN);
    CHECK(opposite(Direction::DOWN)  == Direction::UP);
    CHECK(opposite(Direction::LEFT)  == Direction::RIGHT);
    CHECK(opposite(Direction::RIGHT) == Direction::LEFT);
}

TEST_CASE("Direction: opposite es involutivo") {
    for (Direction d : {Direction::UP, Direction::DOWN,
                        Direction::LEFT, Direction::RIGHT}) {
        CHECK(opposite(opposite(d)) == d);
    }
}

TEST_CASE("Direction: opposite nunca devuelve la misma dirección") {
    for (Direction d : {Direction::UP, Direction::DOWN,
                        Direction::LEFT, Direction::RIGHT}) {
        CHECK(opposite(d) != d);
    }
}