#include "board_fixtures.hpp"

#include "board/board.hpp"
#include <doctest/doctest.h>
#include <cstdint>
#include <vector>

TEST_CASE("Moves: move valido cambia el estado") {
    naidu::Board b(std::uint8_t{3});
    CHECK(b.move(naidu::Direction::LEFT));
    CHECK(b.at(2, 1) == 0);
    CHECK(b.at(2, 2) == 8);
    CHECK(b.empty_index() == 7);
}

TEST_CASE("Moves: move invalido devuelve false y no cambia nada") {
    naidu::Board b(std::uint8_t{3});
    naidu::Board antes = b;
    CHECK_FALSE(b.move(naidu::Direction::DOWN));
    CHECK(b == antes);
}

TEST_CASE("Moves: move en cada direccion") {
    naidu::Board b(std::uint8_t{3});
    b.move(naidu::Direction::UP);
    CHECK(b.move(naidu::Direction::LEFT));
    CHECK(b.move(naidu::Direction::DOWN));
    CHECK(b.move(naidu::Direction::RIGHT));
    CHECK(b.at(2, 2) == 0);
}

TEST_CASE("Moves: legal_moves en el centro devuelve 4") {
    naidu::Board b(std::uint8_t{3});
    b.move(naidu::Direction::UP);
    b.move(naidu::Direction::LEFT);
    auto moves = b.legal_moves();
    CHECK(moves.count() == 4);
}

TEST_CASE("Moves: legal_moves en esquina superior izquierda devuelve 2") {
    naidu::Board b(fixtures::hole_top_left_3x3());
    auto moves = b.legal_moves();
    CHECK(moves.count() == 2);
    CHECK(moves.contains(naidu::Direction::DOWN));
    CHECK(moves.contains(naidu::Direction::RIGHT));
}

TEST_CASE("Moves: legal_moves en borde superior devuelve 3") {
    naidu::Board b(fixtures::hole_top_center_3x3());
    auto moves = b.legal_moves();
    CHECK(moves.count() == 3);
    CHECK_FALSE(moves.contains(naidu::Direction::UP));
}

TEST_CASE("Moves: apply secuencia completa") {
    naidu::Board b(std::uint8_t{3});
    std::vector<naidu::Direction> seq = {
        naidu::Direction::UP,
        naidu::Direction::LEFT,
        naidu::Direction::DOWN
    };
    CHECK(b.apply(seq) == seq.size());
}

TEST_CASE("Moves: apply secuencia que falla a mitad") {
    naidu::Board b(std::uint8_t{3});
    std::vector<naidu::Direction> seq = {
        naidu::Direction::UP,
        naidu::Direction::DOWN,
        naidu::Direction::DOWN
    };
    CHECK(b.apply(seq) == 2);
}

TEST_CASE("Moves: apply secuencia vacia") {
    naidu::Board b(std::uint8_t{3});
    std::vector<naidu::Direction> seq;
    CHECK(b.apply(seq) == 0);
}

TEST_CASE("Moves: reset vuelve al estado resuelto") {
    naidu::Board b(std::uint8_t{4});
    b.move(naidu::Direction::UP);
    b.move(naidu::Direction::LEFT);
    b.move(naidu::Direction::DOWN);
    b.reset();
    fixtures::check_default(b, 4);
}