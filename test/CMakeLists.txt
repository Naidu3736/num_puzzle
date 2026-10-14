find_path(DOCTEST_INCLUDE_DIR doctest/doctest.h)
if(NOT DOCTEST_INCLUDE_DIR)
    message(FATAL_ERROR "No se encontró doctest/doctest.h. ¿Instalaste doctest con pacman?")
endif()

function(add_doctest target_name)
    add_executable(${target_name}
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/test_main.cpp
        ${ARGN}
    )

    target_link_libraries(${target_name} PRIVATE num_puzzle_core)

    target_include_directories(${target_name} PRIVATE
        ${DOCTEST_INCLUDE_DIR}
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_subdirectory(board)
add_subdirectory(direction)
add_subdirectory(moveset)
add_subdirectory(evaluation)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/general/CMakeLists.txt")
    add_subdirectory(general)
endif()