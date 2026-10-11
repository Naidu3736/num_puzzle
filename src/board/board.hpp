#pragma once

#include "core/direction.hpp"
#include "core/moveset.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace naidu {

    /**
     * @brief Representa un tablero de num_puzzle (puzzle deslizante) de tamaño N×N.
     *
     * Almacena las fichas en un arreglo lineal de tamaño N². El valor 0 representa
     * la casilla vacía (hueco). El tablero mantiene su propio índice del hueco
     * para evitar búsquedas repetidas.
     *
     * El almacenamiento interno es un std::array de tamaño fijo MAX_TILES, pero
     * solo los primeros m_size elementos (N²) son válidos. Esto permite cambiar
     * de dimensión en tiempo de ejecución sin asignaciones dinámicas.
     *
     * Dimensiones soportadas: 3×3 hasta 15×15.
     */
    class Board {
    public:
        /** Dimensión mínima permitida del tablero. */
        static constexpr std::uint8_t MIN_DIM   = 3;

        /** Dimensión máxima permitida del tablero. */
        static constexpr std::uint8_t MAX_DIM   = 15;

        /** Número máximo de fichas (15×15 = 225). */
        static constexpr std::uint8_t MAX_TILES = MAX_DIM * MAX_DIM;

    private:
        std::array<std::uint8_t, MAX_TILES> m_board;  ///< Fichas en orden fila-mayor (solo m_size válidas).
        std::uint8_t m_dim;                            ///< Dimensión N del tablero.
        std::uint8_t m_size;                           ///< Número de fichas válidas (N²).
        std::uint8_t m_empty_index;                    ///< Índice lineal del hueco.

        /**
         * @brief Verifica que las fichas formen una permutación válida de 0..N²-1.
         * @throw std::invalid_argument si hay valores duplicados o fuera de rango.
         */
        void validate_tiles() const;

        /**
         * @brief Localiza el índice lineal del hueco (ficha 0).
         * @return Índice lineal del hueco.
         */
        std::uint8_t find_empty() const;

        /**
         * @brief Cuenta el número de inversiones del tablero.
         *
         * Una inversión es un par (i, j) con i < j y tile[i] > tile[j],
         * ignorando el hueco. Se usa para determinar solubilidad.
         *
         * @return Número de inversiones.
         */
        std::size_t inversions() const;

        /**
         * @brief Devuelve el índice del vecino del hueco en la dirección m.
         * @param m Dirección a consultar.
         * @return Índice del vecino, o el propio índice del hueco si no existe.
         */
        std::uint8_t neighbor_of_empty(Direction m) const noexcept;

        /**
         * @brief Comprueba si el hueco puede moverse en la dirección m.
         * @param m Dirección a consultar.
         * @return true si el movimiento es legal.
         */
        bool can_move(Direction m) const noexcept;

    public:
        /**
         * @brief Construye un tablero resuelto de dimensión dim.
         * @param dim Dimensión del tablero (por defecto MIN_DIM).
         */
        explicit Board(std::uint8_t dim = MIN_DIM);

        /**
         * @brief Construye un tablero a partir de una secuencia lineal de fichas.
         * @param tiles Fichas en orden fila-mayor (tamaño debe ser un cuadrado perfecto).
         * @throw std::invalid_argument si el tamaño no es cuadrado o las fichas son inválidas.
         */
        explicit Board(std::span<const std::uint8_t> tiles);

        /**
         * @brief Redimensiona el tablero y lo reinicia a su estado resuelto.
         * @param new_dim Nueva dimensión (entre MIN_DIM y MAX_DIM).
         */
        void resize(std::uint8_t new_dim);

        /**
         * @brief Devuelve la dimensión N del tablero.
         * @return Dimensión N.
         */
        std::uint8_t dim() const noexcept;

        /**
         * @brief Devuelve el número total de fichas (N²).
         * @return Número de fichas.
         */
        std::uint8_t tile_count() const noexcept;

        /**
         * @brief Devuelve el índice lineal del hueco.
         * @return Índice del hueco.
         */
        std::uint8_t empty_index() const noexcept;

        /**
         * @brief Devuelve la ficha en el índice lineal dado.
         * @param index Índice lineal (0..N²-1).
         * @return Valor de la ficha.
         */
        std::uint8_t at(std::size_t index) const;

        /**
         * @brief Devuelve la ficha en la posición (row, col).
         * @param row Fila (0..N-1).
         * @param col Columna (0..N-1).
         * @return Valor de la ficha.
         */
        std::uint8_t at(std::size_t row, std::size_t col) const;

        /**
         * @brief Vista de solo lectura sobre las fichas válidas del tablero.
         *
         * Devuelve un span de tamaño N² (m_size), no de MAX_TILES. Las posiciones
         * no usadas del array interno quedan fuera de la vista.
         *
         * @return Span sobre las N×N fichas.
         */
        std::span<const std::uint8_t> tiles() const noexcept;

        /**
         * @brief Comprueba si el tablero está en el estado resuelto.
         * @return true si todas las fichas están en su posición meta.
         */
        bool is_solved() const noexcept;

        /**
         * @brief Comprueba si el tablero es soluble desde su estado actual.
         *
         * Aplica la regla de paridad: para N impar, el número de inversiones
         * debe ser par; para N par, inversiones + fila del hueco (desde abajo)
         * debe ser impar.
         *
         * @return true si el tablero es soluble.
         */
        bool is_solvable() const noexcept;

        /**
         * @brief Devuelve el conjunto de movimientos legales desde el estado actual.
         * @return MoveSet con las direcciones válidas.
         */
        MoveSet legal_moves() const noexcept;

        /**
         * @brief Intenta mover el hueco en la dirección m.
         * @param m Dirección del movimiento.
         * @return true si el movimiento se realizó, false si era ilegal.
         */
        bool move(Direction m);

        /**
         * @brief Aplica una secuencia de movimientos al tablero.
         * @param sequence Secuencia de direcciones.
         * @return Número de movimientos aplicados con éxito.
         */
        std::size_t apply(std::span<const Direction> sequence);

        /**
         * @brief Reinicia el tablero a su estado resuelto.
         */
        void reset();

        /**
         * @brief Compara dos tableros por contenido y dimensión.
         *
         * Solo compara los primeros m_size elementos. Dos tableros de la misma
         * dimensión con las mismas fichas válidas son iguales, aunque las
         * posiciones no usadas del array interno difieran.
         *
         * @param other Otro tablero.
         * @return true si tienen la misma dimensión y las mismas fichas.
         */
        bool operator==(const Board& other) const noexcept;
    };

}