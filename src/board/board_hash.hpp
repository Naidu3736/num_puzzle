#pragma once

#include "board/board.hpp"

#include <cstddef>
#include <string_view>

namespace naidu {

    /**
     * @brief Hash para Board.
     *
     * Trata las fichas como una secuencia de bytes y delega el hash a
     * std::hash<std::string_view>. Sin copias, distribución uniforme.
     */
    struct BoardHash {
        std::size_t operator()(const Board& b) const noexcept {
            const auto tiles = b.tiles();
            const auto view = std::string_view(
                reinterpret_cast<const char*>(tiles.data()),
                tiles.size()
            );
            return std::hash<std::string_view>{}(view);
        }
    };

}