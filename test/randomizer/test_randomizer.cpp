#include "randomizer/randomizer.hpp"
#include "board/board.hpp"

#include <doctest/doctest.h>

#include <random>

TEST_CASE("Randomizer: respeta la dimensión") {
    std::mt19937 rng(42);

    for (const std::uint8_t dim : {3, 4, 5, 7, 10}) {
        const auto b = naidu::randomizer::generate(dim, rng);
        CHECK(b.dim() == dim);
    }
}

TEST_CASE("Randomizer: genera tableros solubles") {
    std::mt19937 rng(42);

    for (int i = 0; i < 20; ++i) {
        const auto b = naidu::randomizer::generate(3, rng);
        CHECK(b.is_solvable());
    }
}

TEST_CASE("Randomizer: es determinista con la misma semilla") {
    std::mt19937 rng1(42);
    std::mt19937 rng2(42);

    const auto b1 = naidu::randomizer::generate(3, rng1);
    const auto b2 = naidu::randomizer::generate(3, rng2);

    CHECK(b1 == b2);
}

TEST_CASE("Randomizer: genera tableros distintos con distinta semilla") {
    std::mt19937 rng1(42);
    std::mt19937 rng2(43);

    const auto b1 = naidu::randomizer::generate(3, rng1);
    const auto b2 = naidu::randomizer::generate(3, rng2);

    CHECK(b1 != b2);
}

TEST_CASE("Randomizer: sin avoid_revisits funciona") {
    std::mt19937 rng(42);

    naidu::randomizer::Config config{
        .moves = 100,
        .avoid_revisits = false,
    };

    const auto b = naidu::randomizer::generate(3, config, rng);
    CHECK(b.dim() == 3);
    CHECK(b.is_solvable());
}

TEST_CASE("Randomizer: con avoid_revisits funciona") {
    std::mt19937 rng(42);

    naidu::randomizer::Config config{
        .moves = 100,
        .avoid_revisits = true,
    };

    const auto b = naidu::randomizer::generate(3, config, rng);
    CHECK(b.dim() == 3);
    CHECK(b.is_solvable());
}

TEST_CASE("Randomizer: no se atasca con avoid_revisits") {
    std::mt19937 rng(42);

    naidu::randomizer::Config config{
        .moves = 10000,
        .avoid_revisits = true,
    };

    const auto b = naidu::randomizer::generate(3, config, rng);
    CHECK(b.dim() == 3);
}