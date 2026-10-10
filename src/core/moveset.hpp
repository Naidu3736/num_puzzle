#pragma once

#include "direction.hpp"
#include <cstdint>
#include <bit>

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