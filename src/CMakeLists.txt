add_library(num_puzzle_core STATIC
    core/direction.hpp
    core/moveset.hpp

    board/board.hpp
    board/board.cpp

    evaluation/evaluation.hpp
    evaluation/evaluation.cpp
    
    evaluation/component.hpp
    evaluation/component.cpp

    solver/solver.hpp
    solver/solver.cpp

    # randomizer/randomizer.hpp
    # randomizer/randomizer.cpp
)

target_include_directories(num_puzzle_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(num_puzzle_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

qt_add_executable(num_puzzle
    main.cpp

    ui/mainwindow.h
    ui/mainwindow.cpp
    ui/mainwindow.ui
)

target_link_libraries(num_puzzle PRIVATE
    num_puzzle_core
    Qt6::Widgets
)