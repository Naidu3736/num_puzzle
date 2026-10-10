#include <doctest/doctest.h>

#include "core/moveset.hpp"
#include <initializer_list>

using naidu::Direction;
using naidu::MoveSet;

TEST_CASE("MoveSet: por defecto está vacío") {
    MoveSet ms;
    CHECK(ms.empty());
    CHECK(ms.count() == 0);
    CHECK(ms.raw() == 0);
    CHECK_FALSE(ms.contains(Direction::UP));
    CHECK_FALSE(ms.contains(Direction::DOWN));
    CHECK_FALSE(ms.contains(Direction::LEFT));
    CHECK_FALSE(ms.contains(Direction::RIGHT));
}

TEST_CASE("MoveSet: from crea un set con un solo movimiento") {
    for (Direction d : {Direction::UP, Direction::DOWN,
                        Direction::LEFT, Direction::RIGHT}) {
        const MoveSet ms = MoveSet::from(d);
        CHECK_FALSE(ms.empty());
        CHECK(ms.count() == 1);
        CHECK(ms.contains(d));
    }
}

TEST_CASE("MoveSet: from asigna un bit distinto por dirección") {
    CHECK(MoveSet::from(Direction::UP).raw()    == 0b0001);
    CHECK(MoveSet::from(Direction::DOWN).raw()  == 0b0010);
    CHECK(MoveSet::from(Direction::LEFT).raw()  == 0b0100);
    CHECK(MoveSet::from(Direction::RIGHT).raw() == 0b1000);
}

TEST_CASE("MoveSet: operator| (MoveSet, Direction) combina") {
    MoveSet ms = MoveSet::from(Direction::UP) | Direction::DOWN;
    CHECK(ms.count() == 2);
    CHECK(ms.contains(Direction::UP));
    CHECK(ms.contains(Direction::DOWN));
    CHECK_FALSE(ms.contains(Direction::LEFT));
    CHECK_FALSE(ms.contains(Direction::RIGHT));
}

TEST_CASE("MoveSet: operator| (Direction, MoveSet) combina") {
    const MoveSet base = MoveSet::from(Direction::UP);
    MoveSet ms = Direction::DOWN | base;
    CHECK(ms.count() == 2);
    CHECK(ms.contains(Direction::UP));
    CHECK(ms.contains(Direction::DOWN));
}

TEST_CASE("MoveSet: operator| (Direction, Direction) combina") {
    MoveSet ms = Direction::UP | Direction::LEFT;
    CHECK(ms.count() == 2);
    CHECK(ms.contains(Direction::UP));
    CHECK(ms.contains(Direction::LEFT));
    CHECK_FALSE(ms.contains(Direction::DOWN));
    CHECK_FALSE(ms.contains(Direction::RIGHT));
}

TEST_CASE("MoveSet: operator| (MoveSet, MoveSet) une") {
    const MoveSet a = MoveSet::from(Direction::UP) | Direction::DOWN;
    const MoveSet b = MoveSet::from(Direction::LEFT) | Direction::RIGHT;
    const MoveSet c = a | b;
    CHECK(c.count() == 4);
    CHECK(c.raw() == 0b1111);
}

TEST_CASE("MoveSet: operator| es conmutativo") {
    const MoveSet a = Direction::UP | Direction::LEFT;
    const MoveSet b = Direction::LEFT | Direction::UP;
    CHECK(a.raw() == b.raw());
}

TEST_CASE("MoveSet: operator| es idempotente") {
    const MoveSet a = MoveSet::from(Direction::UP);
    CHECK((a | Direction::UP).raw() == a.raw());
    CHECK((a | a).raw() == a.raw());
}

TEST_CASE("MoveSet: operator| acumula todos los movimientos") {
    MoveSet ms;
    ms = ms | Direction::UP;
    ms = ms | Direction::DOWN;
    ms = ms | Direction::LEFT;
    ms = ms | Direction::RIGHT;

    CHECK(ms.count() == 4);
    CHECK_FALSE(ms.empty());
    CHECK(ms.raw() == 0b1111);
    CHECK(ms.contains(Direction::UP));
    CHECK(ms.contains(Direction::DOWN));
    CHECK(ms.contains(Direction::LEFT));
    CHECK(ms.contains(Direction::RIGHT));
}

TEST_CASE("MoveSet: operator& sin intersección da vacío") {
    const MoveSet a = MoveSet::from(Direction::UP);
    const MoveSet b = MoveSet::from(Direction::DOWN);
    CHECK((a & b).empty());
    CHECK((a & b).raw() == 0);
}

TEST_CASE("MoveSet: operator& con intersección no vacía") {
    const MoveSet a = MoveSet::from(Direction::UP) | Direction::DOWN;
    const MoveSet b = MoveSet::from(Direction::DOWN) | Direction::LEFT;
    const MoveSet c = a & b;
    CHECK(c.count() == 1);
    CHECK(c.contains(Direction::DOWN));
    CHECK_FALSE(c.contains(Direction::UP));
    CHECK_FALSE(c.contains(Direction::LEFT));
}

TEST_CASE("MoveSet: operator& es conmutativo e idempotente") {
    const MoveSet a = MoveSet::from(Direction::UP) | Direction::DOWN;
    const MoveSet b = MoveSet::from(Direction::DOWN) | Direction::LEFT;
    CHECK((a & b).raw() == (b & a).raw());
    CHECK((a & a).raw() == a.raw());
}

TEST_CASE("MoveSet: operator& con vacío da vacío") {
    const MoveSet a = MoveSet::from(Direction::UP) | Direction::DOWN;
    const MoveSet vacio;
    CHECK((a & vacio).empty());
    CHECK((vacio & a).empty());
}

TEST_CASE("MoveSet: count refleja el número de bits activos") {
    CHECK(MoveSet::from(Direction::UP).count() == 1);
    CHECK((MoveSet::from(Direction::UP) | Direction::DOWN).count() == 2);
    CHECK((MoveSet::from(Direction::UP) | Direction::DOWN
           | Direction::LEFT).count() == 3);
    CHECK((MoveSet::from(Direction::UP) | Direction::DOWN
           | Direction::LEFT | Direction::RIGHT).count() == 4);
}

TEST_CASE("MoveSet: usable en contexto constexpr") {
    constexpr MoveSet s = Direction::UP | Direction::DOWN;
    static_assert(s.count() == 2);
    static_assert(s.contains(Direction::UP));
    static_assert(s.contains(Direction::DOWN));
    static_assert(!s.contains(Direction::LEFT));
    CHECK(true);
}