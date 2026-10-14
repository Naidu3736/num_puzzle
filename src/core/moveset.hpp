#pragma once

#include "direction.hpp"
#include <bit>
#include <cstdint>

namespace naidu {

    /**
     * @brief Conjunto de direcciones, representado como una máscara de bits.
     *
     * Cada bit del índice i indica si la dirección i-ésima está presente
     * (ver Direction y to_index). Permite combinar y consultar movimientos
     * legales de forma O(1) y sin asignaciones dinámicas.
     *
     * Ejemplo:
     *   MoveSet s = Direction::UP | Direction::LEFT;
     *   s.contains(Direction::UP);    // true
     *   s.count();                    // 2
     *   for (Direction d : s) { ... }
     */
    class MoveSet {
        std::uint8_t m_mask = 0;   ///< Máscara de bits: bit i ↔ dirección i.

        /**
         * @brief Construye un MoveSet a partir de una máscara cruda.
         * @param m Máscara de bits (bits 0..3 relevantes).
         */
        constexpr explicit MoveSet(std::uint8_t m) noexcept : m_mask(m) {}

        friend constexpr MoveSet operator|(Direction, Direction) noexcept;
        friend constexpr MoveSet operator|(MoveSet, Direction) noexcept;
        friend constexpr MoveSet operator|(MoveSet, MoveSet) noexcept;
        friend constexpr MoveSet operator&(MoveSet, MoveSet) noexcept;

    public:
        /** @brief Construye un MoveSet vacío. */
        constexpr MoveSet() noexcept = default;

        /**
         * @brief Crea un MoveSet con una sola dirección.
         * @param d Dirección a incluir.
         * @return MoveSet con el bit correspondiente activado.
         */
        static constexpr MoveSet from(Direction d) noexcept {
            return MoveSet(std::uint8_t{1} << to_index(d));
        }

        /**
         * @brief Comprueba si una dirección está contenida en el conjunto.
         * @param d Dirección a consultar.
         * @return true si el bit correspondiente está activo.
         */
        constexpr bool contains(Direction d) const noexcept {
            return (m_mask >> to_index(d)) & 1u;
        }

        /**
         * @brief Comprueba si el conjunto está vacío.
         * @return true si no contiene ninguna dirección.
         */
        constexpr bool empty() const noexcept { return m_mask == 0; }

        /**
         * @brief Cuenta cuántas direcciones contiene el conjunto.
         * @return Número de bits activos (0..4).
         */
        constexpr std::uint8_t count() const noexcept {
            return static_cast<std::uint8_t>(std::popcount(m_mask));
        }

        /**
         * @brief Devuelve la máscara de bits subyacente.
         * @return Máscara cruda (bits 0..3).
         */
        constexpr std::uint8_t raw() const noexcept { return m_mask; }

        // --------------------------------------------------------
        // Iterador
        // --------------------------------------------------------

        /**
         * @brief Iterador minimalista sobre las direcciones activas.
         *
         * Salta los bits apagados. Compatible con el bucle `for range`:
         *
         *   for (Direction d : moves) { ... }
         */
        class iterator {
            std::uint8_t m_mask;   ///< Máscara del MoveSet.
            std::uint8_t m_index;  ///< Índice actual (0..direction_count).

            /**
             * @brief Avanza m_index hasta el siguiente bit activo.
             *
             * Si no hay más bits activos, deja m_index en direction_count,
             * lo cual es equivalente a `end()`.
             */
            void advance_to_active() {
                while (m_index < 4 && !((m_mask >> m_index) & 1u))
                    ++m_index;
            }

        public:
            /**
             * @brief Construye un iterador sobre la máscara dada.
             * @param mask Máscara del MoveSet.
             * @param index Índice inicial (0..4).
             */
            iterator(std::uint8_t mask, std::uint8_t index)
                : m_mask(mask), m_index(index) {
                advance_to_active();
            }

            /**
             * @brief Dirección actual.
             * @return Direction correspondiente al bit activo actual.
             */
            Direction operator*() const {
                return static_cast<Direction>(m_index);
            }

            /**
             * @brief Avanza al siguiente bit activo.
             * @return Referencia al iterador actualizado.
             */
            iterator& operator++() {
                ++m_index;
                advance_to_active();
                return *this;
            }

            /**
             * @brief Compara dos iteradores por su índice.
             * @param other Otro iterador.
             * @return true si apuntan a índices distintos.
             */
            bool operator!=(const iterator& other) const {
                return m_index != other.m_index;
            }
        };

        /**
         * @brief Iterador al primer bit activo.
         * @return iterator sobre el primer bit activo (o end() si vacío).
         */
        iterator begin() const {
            return iterator(m_mask, 0);
        }

        /**
         * @brief Iterador al final (uno más allá del último bit).
         * @return iterator con m_index == 4 (fin).
         */
        iterator end() const {
            return iterator(m_mask, 4);
        }
    };

    /**
     * @brief Unión de dos direcciones.
     * @param a Primera dirección.
     * @param b Segunda dirección.
     * @return MoveSet con ambas direcciones.
     */
    constexpr MoveSet operator|(Direction a, Direction b) noexcept {
        return MoveSet(MoveSet::from(a).raw() | MoveSet::from(b).raw());
    }

    /**
     * @brief Añade una dirección a un MoveSet.
     * @param a MoveSet original.
     * @param b Dirección a añadir.
     * @return Nuevo MoveSet con la dirección añadida.
     */
    constexpr MoveSet operator|(MoveSet a, Direction b) noexcept {
        return MoveSet(a.raw() | MoveSet::from(b).raw());
    }

    /**
     * @brief Añade un MoveSet a una dirección (conmutativo).
     * @param a Dirección.
     * @param b MoveSet.
     * @return Nuevo MoveSet con ambos contenidos.
     */
    constexpr MoveSet operator|(Direction a, MoveSet b) noexcept { return b | a; }

    /**
     * @brief Unión de dos MoveSets.
     * @param a Primer MoveSet.
     * @param b Segundo MoveSet.
     * @return MoveSet con todas las direcciones de ambos.
     */
    constexpr MoveSet operator|(MoveSet a, MoveSet b) noexcept {
        return MoveSet(a.raw() | b.raw());
    }

    /**
     * @brief Intersección de dos MoveSets.
     * @param a Primer MoveSet.
     * @param b Segundo MoveSet.
     * @return MoveSet con las direcciones comunes a ambos.
     */
    constexpr MoveSet operator&(MoveSet a, MoveSet b) noexcept {
        return MoveSet(a.raw() & b.raw());
    }

}