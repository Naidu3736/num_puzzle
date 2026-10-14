#include "board_fixtures.hpp"
#include "board/board.hpp"
#include "solver/solver.hpp"
#include "evaluation/evaluation.hpp"

#include <doctest/doctest.h>

#include <iostream>

namespace {

    char direction_to_char(naidu::Direction d) {
        switch (d) {
            case naidu::Direction::UP:    return 'U';
            case naidu::Direction::DOWN:  return 'D';
            case naidu::Direction::LEFT:  return 'L';
            case naidu::Direction::RIGHT: return 'R';
        }
        return '?';
    }

    void print_solution(const naidu::Solution& sol) {
        std::cout << "  found: " << (sol.found ? "true" : "false") << "\n";
        std::cout << "  cost: " << sol.cost << "\n";
        std::cout << "  nodes: " << sol.nodes_expanded << "\n";
        std::cout << "  moves: ";
        for (const auto d : sol.moves)
            std::cout << direction_to_char(d) << ",";
        std::cout << "\n";
    }

}

TEST_CASE("Solver: la secuencia de movimientos resuelve el tablero") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1
    };

    const auto sol = naidu::solver(initial, goal, config);

    CHECK(sol.found);
    
    initial.apply(sol.moves);
    CHECK(initial == goal);
}

TEST_CASE("Solver: tablero resuelto devuelve camino vacío") {
    naidu::Board initial(3);
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
    };

    const auto sol = naidu::solver(initial, goal, config);
    initial.apply(sol.moves);
    // print_solution(sol);
    
    CHECK(sol.found);
    CHECK(sol.moves.empty());
    CHECK(sol.cost == 0);
}

TEST_CASE("Solver: un solo movimiento") {
    naidu::Board initial({
        1, 2, 3,
        4, 5, 6,
        7, 0, 8,
    });
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
    CHECK(sol.moves.size() == 1);
    CHECK(sol.cost == 1);
}

TEST_CASE("Solver: tablero 3x3 revuelto") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
    CHECK(!sol.moves.empty());
}

TEST_CASE("Solver: respeta max_nodes (0 = sin límite)") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .max_nodes = 0,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: límite muy bajo devuelve not found") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .max_nodes = 1,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK_FALSE(sol.found);
}

TEST_CASE("Solver: avoid_revisits = false funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .avoid_revisits = false,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: avoid_revisits = true funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .avoid_revisits = true,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: weighted_cost = false funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .weighted_cost = false,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: weighted_cost = true funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
        .weighted_cost = true,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: evaluation_2 funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_2,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: evaluation_3 funciona") {
    naidu::Board initial(fixtures::random_3x3_1());
    naidu::Board goal(3);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_3,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
}

TEST_CASE("Solver: 4x4 a un movimiento") {
    naidu::Board initial({
        1,  2,  3,  4,
        5,  6,  7,  8,
        9, 10, 11, 12,
       13, 14,  0, 15,
    });
    naidu::Board goal(4);

    naidu::Config config{
        .eval_fn = naidu::evaluation::evaluation_1,
    };

    const auto sol = naidu::solver(initial, goal, config);
    // print_solution(sol);

    CHECK(sol.found);
    CHECK(sol.moves.size() == 1);
}