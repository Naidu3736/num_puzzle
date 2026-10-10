#include "board.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <bitset>

namespace naidu {

    std::size_t Board::inversions() const {
        std::size_t inv = 0;
        const std::size_t n = m_board.size();

        for (std::size_t i = 0; i + 1 < n; ++i) {
            if (m_board[i] == 0) continue;
            for (std::size_t j = i + 1; j < n; ++j) {
                if (m_board[j] == 0) continue;
                if (m_board[i] > m_board[j]) ++inv;
            }
        }
        return inv;
    }

    void Board::validate_tiles() const {
        std::bitset<MAX_TILES> seen;
        for (std::uint8_t tile : m_board) {
            if (tile >= m_board.size())
                throw std::invalid_argument("Tile out of range");
            if (seen.test(tile))
                throw std::invalid_argument("Duplicate tile value");
            seen.set(tile);
        }
    }

    std::uint8_t Board::find_empty() const {
        auto it = std::find(m_board.begin(), m_board.end(), std::uint8_t{0});
        if (it == m_board.end())
            throw std::invalid_argument("Board does not contain an empty tile");
        return static_cast<std::uint8_t>(it - m_board.begin());
    }

    std::uint8_t Board::neighbor_of_empty(Direction m) const noexcept {
        switch (m) {
            case Direction::UP:    return static_cast<std::uint8_t>(m_empty_index - m_dim);
            case Direction::DOWN:  return static_cast<std::uint8_t>(m_empty_index + m_dim);
            case Direction::LEFT:  return static_cast<std::uint8_t>(m_empty_index - 1);
            case Direction::RIGHT: return static_cast<std::uint8_t>(m_empty_index + 1);
        }
        return m_empty_index;
    }

    bool Board::can_move(Direction m) const noexcept {
        const std::uint8_t row = m_empty_index / m_dim;
        const std::uint8_t col = m_empty_index % m_dim;

        switch (m) {
            case Direction::UP:    return row > 0;
            case Direction::DOWN:  return row < m_dim - 1;
            case Direction::LEFT:  return col > 0;
            case Direction::RIGHT: return col < m_dim - 1;
        }
        return false;
    }

    Board::Board(std::uint8_t dim) {
        resize(dim);
    }

    Board::Board(std::span<const std::uint8_t> tiles) {
        const std::size_t n = tiles.size();
        const auto dim = static_cast<std::uint8_t>(std::lround(std::sqrt(static_cast<double>(n))));

        if (static_cast<std::size_t>(dim) * dim != n)
            throw std::invalid_argument("Board size is not a perfect square");
        if (dim < MIN_DIM || dim > MAX_DIM)
            throw std::invalid_argument("Board dimension must be between 3 and 15");

        m_dim = dim;
        m_board.assign(tiles.begin(), tiles.end());
        validate_tiles();
        m_empty_index = find_empty();
    }

    void Board::resize(std::uint8_t new_dim) {
        if (new_dim < MIN_DIM)
            throw std::invalid_argument("Board dimension must be at least 3");
        if (new_dim > MAX_DIM)
            throw std::invalid_argument("Board dimension must be at most 15");

        m_dim = new_dim;
        reset();
    }

    std::uint8_t Board::dim() const noexcept          { return m_dim; }
    std::uint8_t Board::tile_count() const noexcept   { return static_cast<std::uint8_t>(m_board.size()); }
    std::uint8_t Board::empty_index() const noexcept  { return m_empty_index; }

    std::uint8_t Board::at(std::size_t index) const  { return m_board[index]; }
    std::uint8_t Board::at(std::size_t row, std::size_t col) const {
        return m_board[row * m_dim + col];
    }

    std::span<const std::uint8_t> Board::tiles() const noexcept { return m_board; }

    bool Board::is_solved() const noexcept {
        for (std::uint8_t i = 0; i + 1 < m_board.size(); ++i)
            if (m_board[i] != static_cast<std::uint8_t>(i + 1)) return false;
        return m_board.back() == 0;
    }

    bool Board::is_solvable() const noexcept {
        const std::size_t inv = inversions();
        if (m_dim & 1) return !(inv & 1);
        const std::size_t row = m_empty_index / m_dim;
        return (inv + row) & 1;
    }

    MoveSet Board::legal_moves() const noexcept {
        MoveSet r;
        if (can_move(Direction::UP))    r = r | Direction::UP;
        if (can_move(Direction::DOWN))  r = r | Direction::DOWN;
        if (can_move(Direction::LEFT))  r = r | Direction::LEFT;
        if (can_move(Direction::RIGHT)) r = r | Direction::RIGHT;
        return r;
    }

    bool Board::move(Direction m) {
        if (!can_move(m)) return false;
        const std::uint8_t neighbor = neighbor_of_empty(m);
        std::swap(m_board[m_empty_index], m_board[neighbor]);
        m_empty_index = neighbor;
        return true;
    }

    std::size_t Board::apply(std::span<const Direction> sequence) {
        for (std::size_t i = 0; i < sequence.size(); ++i)
            if (!move(sequence[i])) return i;
        return sequence.size();
    }

    void Board::reset() {
        m_board.resize(static_cast<std::size_t>(m_dim) * m_dim);
        for (std::uint8_t i = 0; i + 1 < m_board.size(); ++i)
            m_board[i] = static_cast<std::uint8_t>(i + 1);
        m_board.back() = 0;
        m_empty_index = static_cast<std::uint8_t>(m_board.size() - 1);
    }

    bool Board::operator==(const Board& other) const noexcept {
        return m_dim == other.m_dim
            && m_empty_index == other.m_empty_index
            && m_board == other.m_board;
    }

}