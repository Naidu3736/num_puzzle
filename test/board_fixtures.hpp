#pragma once

#include "board/board.hpp"
#include <doctest/doctest.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace fixtures {

    inline void check_default(const naidu::Board& b, std::size_t dim) {
        CHECK(b.dim() == dim);

        for (std::size_t i = 0; i < b.tile_count(); ++i) {
            if (i == b.tile_count() - 1)
                CHECK(b.at(i) == 0);
            else
                CHECK(b.at(i) == i + 1);
        }

        CHECK(b.empty_index() == b.tile_count() - 1);
        CHECK(b.is_solved());
    }

    // --- Tableros 4x4 reutilizables ---

    inline std::vector<std::uint8_t> solved_4x4() {
        return {
            1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10, 11, 12,
            13, 14, 15,  0
        };
    }

    // El 0 en la esquina pero con 14 y 15 intercambiados: no resuelto, no resoluble.
    inline std::vector<std::uint8_t> unsolvable_4x4() {
        return {
            1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10, 11, 12,
            13, 15, 14,  0
        };
    }

    // 0 en el centro, con 11 y 15 en posiciones raras: resoluble.
    inline std::vector<std::uint8_t> solvable_hole_center_4x4() {
        return {
            1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10,  0, 12,
            13, 14, 11, 15
        };
    }

    // El 0 no está al final: no resuelto pero resoluble.
    inline std::vector<std::uint8_t> solvable_unsolved_4x4() {
        return {
            1,  2,  3,  4,
            5,  6,  7,  8,
            9, 10, 11, 12,
            13, 14,  0, 15
        };
    }

    // --- Tableros 5x5 reutilizables ---

    // 5x5 con 0 en (4,3): resoluble (impar).
    inline std::vector<std::uint8_t> solvable_5x5() {
        return {
            1,  2,  3,  4,  5,
            6,  7,  8,  9, 10,
            11, 12, 13, 14, 15,
            16, 17, 18, 19, 20,
            21, 22, 23,  0, 24
        };
    }

    // 5x5 con 19 y 20 intercambiados y 0 al final: no resoluble.
    inline std::vector<std::uint8_t> unsolvable_5x5() {
        return {
            1,  2,  3,  4,  5,
            6,  7,  8,  9, 10,
            11, 12, 13, 14, 15,
            16, 17, 18, 20, 19,
            21, 22, 23, 24,  0
        };
    }

    // --- Tableros 3x3 reutilizables ---

    inline std::vector<std::uint8_t> hole_top_left_3x3() {
        return {
            0,  1,  2,
            3,  4,  5,
            6,  7,  8
        };
    }

    inline std::vector<std::uint8_t> hole_top_center_3x3() {
        return {
            1,  0,  2,
            3,  4,  5,
            6,  7,  8
        };
    }

    // --- Tableros aleatorios predefinidos ---
    // Manhattan calculado pieza por pieza, sin contar el 0.
    // broken_pairs = número de pares adyacentes mal ordenados (sin contar el 0).
    // tiles_at_1_distance: piezas mal colocadas a Manhattan 1 (sin contar el 0).
    // blank_dispersion: distancia Manhattan media del hueco a cada ficha mal colocada.

    // 3x3 aleatorio #1: resoluble, 0 en (2,1).
    // Manhattan = 21, bad_tiles = 7, broken_pairs = 10, tiles_at_1 = 0, dispersion = 1.875
    inline std::vector<std::uint8_t> random_3x3_1() {
        return {
            8,  6,  7,
            2,  5,  4,
            3,  0,  1,
        };
    }
    inline constexpr std::uint16_t manhattan_random_3x3_1 = 21;
    inline constexpr std::uint16_t bad_tiles_random_3x3_1 = 7;
    inline constexpr std::uint16_t broken_pairs_random_3x3_1 = 10;
    inline constexpr std::uint16_t tiles_at_1_distance_random_3x3_1 = 0;
    inline constexpr double blank_dispersion_random_3x3_1 = 2;

    // 3x3 aleatorio #2: resoluble, 0 en (2,1).
    // Manhattan = 11, bad_tiles = 8, broken_pairs = 10, tiles_at_1 = 6, dispersion = 1.875
    inline std::vector<std::uint8_t> random_3x3_2() {
        return {
            3,  1,  4,
            7,  2,  5,
            8,  0,  6,
        };
    }
    inline constexpr std::uint16_t manhattan_random_3x3_2 = 11;
    inline constexpr std::uint16_t bad_tiles_random_3x3_2 = 8;
    inline constexpr std::uint16_t broken_pairs_random_3x3_2 = 10;
    inline constexpr std::uint16_t tiles_at_1_distance_random_3x3_2 = 6;
    inline constexpr double blank_dispersion_random_3x3_2 = 1.875;

    // 3x3 aleatorio #3: resoluble, 0 en (0,0).
    // Manhattan = 8, bad_tiles = 8, broken_pairs = 8, tiles_at_1 = 8, dispersion = 2.25
    inline std::vector<std::uint8_t> random_3x3_3() {
        return {
            0,  5,  2,
            1,  8,  3,
            4,  7,  6,
        };
    }
    inline constexpr std::uint16_t manhattan_random_3x3_3 = 8;
    inline constexpr std::uint16_t bad_tiles_random_3x3_3 = 8;
    inline constexpr std::uint16_t broken_pairs_random_3x3_3 = 8;
    inline constexpr std::uint16_t tiles_at_1_distance_random_3x3_3 = 8;
    inline constexpr double blank_dispersion_random_3x3_3 = 2.25;

    // 4x4 aleatorio #1: resoluble, 0 en (2,2).
    // Manhattan = 10, bad_tiles = 9, broken_pairs = 14, tiles_at_1 = 8, dispersion = 2.333
    inline std::vector<std::uint8_t> random_4x4_1() {
        return {
            5,   1,   3,   4,
            9,   2,   7,   8,
            13,  6,   0,   12,
            10,  14,  11,  15,
        };
    }
    inline constexpr std::uint16_t manhattan_random_4x4_1 = 10;
    inline constexpr std::uint16_t bad_tiles_random_4x4_1 = 9;
    inline constexpr std::uint16_t broken_pairs_random_4x4_1 = 14;
    inline constexpr std::uint16_t tiles_at_1_distance_random_4x4_1 = 8;
    inline constexpr double blank_dispersion_random_4x4_1 = 2.333333333;

    // 4x4 aleatorio #2: resoluble, 0 en (3,2).
    // Manhattan = 13, bad_tiles = 9, broken_pairs = 14, tiles_at_1 = 6, dispersion = 2.444
    inline std::vector<std::uint8_t> random_4x4_2() {
        return {
            2,   8,   3,   4,
            1,   6,   7,   12,
            5,   10,  11,  15,
            9,   13,  0,   14,
        };
    }
    inline constexpr std::uint16_t manhattan_random_4x4_2 = 13;
    inline constexpr std::uint16_t bad_tiles_random_4x4_2 = 9;
    inline constexpr std::uint16_t broken_pairs_random_4x4_2 = 14;
    inline constexpr std::uint16_t tiles_at_1_distance_random_4x4_2 = 6;
    inline constexpr double blank_dispersion_random_4x4_2 = 2.777777778;

    // 4x4 aleatorio #3: resoluble, 0 en (0,3).
    // Manhattan = 3, bad_tiles = 3, broken_pairs = 4, tiles_at_1 = 3, dispersion = 2.0
    inline std::vector<std::uint8_t> random_4x4_3() {
        return {
            1,   2,   3,   0,
            5,   6,   7,   4,
            9,   10,  11,  8,
            13,  14,  15,  12,
        };
    }
    inline constexpr std::uint16_t manhattan_random_4x4_3 = 3;
    inline constexpr std::uint16_t bad_tiles_random_4x4_3 = 3;
    inline constexpr std::uint16_t broken_pairs_random_4x4_3 = 4;
    inline constexpr std::uint16_t tiles_at_1_distance_random_4x4_3 = 3;
    inline constexpr double blank_dispersion_random_4x4_3 = 2.0;

    // 5x5 aleatorio #1: resoluble, 0 en (2,2).
    // Manhattan = 4, bad_tiles = 1, broken_pairs = 2, tiles_at_1 = 0, dispersion = 4.0
    inline std::vector<std::uint8_t> random_5x5_1() {
        return {
            1,   2,   3,   4,   5,
            6,   7,   8,   9,   10,
            11,  12,  0,   14,  15,
            16,  17,  18,  19,  20,
            21,  22,  23,  24,  13,
        };
    }
    inline constexpr std::uint16_t manhattan_random_5x5_1 = 4;
    inline constexpr std::uint16_t bad_tiles_random_5x5_1 = 1;
    inline constexpr std::uint16_t broken_pairs_random_5x5_1 = 2;
    inline constexpr std::uint16_t tiles_at_1_distance_random_5x5_1 = 0;
    inline constexpr double blank_dispersion_random_5x5_1 = 4.0;

    // 5x5 aleatorio #2: resoluble, 0 en (4,0).
    // Manhattan = 4, bad_tiles = 4, broken_pairs = 4, tiles_at_1 = 4, dispersion = 2.5
    inline std::vector<std::uint8_t> random_5x5_2() {
        return {
            1,   2,   3,   4,   5,
            6,   7,   8,   9,   10,
            11,  12,  13,  14,  15,
            16,  17,  18,  19,  20,
            0,   21,  22,  23,  24,
        };
    }
    inline constexpr std::uint16_t manhattan_random_5x5_2 = 4;
    inline constexpr std::uint16_t bad_tiles_random_5x5_2 = 4;
    inline constexpr std::uint16_t broken_pairs_random_5x5_2 = 4;
    inline constexpr std::uint16_t tiles_at_1_distance_random_5x5_2 = 4;
    inline constexpr double blank_dispersion_random_5x5_2 = 2.5;

    // 5x5 aleatorio #3: resoluble, 0 en (1,3).
    // Manhattan = 4, bad_tiles = 1, broken_pairs = 1, tiles_at_1 = 0, dispersion = 4.0
    inline std::vector<std::uint8_t> random_5x5_3() {
        return {
            1,   2,   3,   4,   5,
            6,   7,   8,   0,   10,
            11,  12,  13,  14,  15,
            16,  17,  18,  19,  20,
            21,  22,  23,  24,  9,
        };
    }
    inline constexpr std::uint16_t manhattan_random_5x5_3 = 4;
    inline constexpr std::uint16_t bad_tiles_random_5x5_3 = 1;
    inline constexpr std::uint16_t broken_pairs_random_5x5_3 = 1;
    inline constexpr std::uint16_t tiles_at_1_distance_random_5x5_3 = 0;
    inline constexpr double blank_dispersion_random_5x5_3 = 4.0;

    // ---------------------------------------------------------------------------
    // Estructura y mapa de casos
    // ---------------------------------------------------------------------------

    struct BoardCase {
        std::vector<std::uint8_t> tiles;
        std::uint16_t manhattan;
        std::uint16_t bad_tiles;
        std::uint16_t broken_pairs;
        std::uint16_t tiles_at_1_distance;
        double blank_dispersion;
    };

    inline const std::map<std::string, BoardCase> random_cases = {
        {"random_3x3_1", {random_3x3_1(), manhattan_random_3x3_1, bad_tiles_random_3x3_1,
                          broken_pairs_random_3x3_1, tiles_at_1_distance_random_3x3_1,
                          blank_dispersion_random_3x3_1}},
        {"random_3x3_2", {random_3x3_2(), manhattan_random_3x3_2, bad_tiles_random_3x3_2,
                          broken_pairs_random_3x3_2, tiles_at_1_distance_random_3x3_2,
                          blank_dispersion_random_3x3_2}},
        {"random_3x3_3", {random_3x3_3(), manhattan_random_3x3_3, bad_tiles_random_3x3_3,
                          broken_pairs_random_3x3_3, tiles_at_1_distance_random_3x3_3,
                          blank_dispersion_random_3x3_3}},
        {"random_4x4_1", {random_4x4_1(), manhattan_random_4x4_1, bad_tiles_random_4x4_1,
                          broken_pairs_random_4x4_1, tiles_at_1_distance_random_4x4_1,
                          blank_dispersion_random_4x4_1}},
        {"random_4x4_2", {random_4x4_2(), manhattan_random_4x4_2, bad_tiles_random_4x4_2,
                          broken_pairs_random_4x4_2, tiles_at_1_distance_random_4x4_2,
                          blank_dispersion_random_4x4_2}},
        {"random_4x4_3", {random_4x4_3(), manhattan_random_4x4_3, bad_tiles_random_4x4_3,
                          broken_pairs_random_4x4_3, tiles_at_1_distance_random_4x4_3,
                          blank_dispersion_random_4x4_3}},
        {"random_5x5_1", {random_5x5_1(), manhattan_random_5x5_1, bad_tiles_random_5x5_1,
                          broken_pairs_random_5x5_1, tiles_at_1_distance_random_5x5_1,
                          blank_dispersion_random_5x5_1}},
        {"random_5x5_2", {random_5x5_2(), manhattan_random_5x5_2, bad_tiles_random_5x5_2,
                          broken_pairs_random_5x5_2, tiles_at_1_distance_random_5x5_2,
                          blank_dispersion_random_5x5_2}},
        {"random_5x5_3", {random_5x5_3(), manhattan_random_5x5_3, bad_tiles_random_5x5_3,
                          broken_pairs_random_5x5_3, tiles_at_1_distance_random_5x5_3,
                          blank_dispersion_random_5x5_3}},
    };


} // namespace fixtures