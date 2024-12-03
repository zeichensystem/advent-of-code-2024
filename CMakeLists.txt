# Select debug/release variant: % cmake -DCMAKE_BUILD_TYPE=Debug/Release ../..
# Build and run: make run-day-XX

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE OR UNIX OR LINUX)
    set(WARNING_FLAGS_CXX -Wall -Wextra -Wpedantic -Wvla -Wshadow -Wundef -Wmisleading-indentation -Wnull-dereference -Wshadow  -Wundef -Wstrict-overflow=5 -Wsign-promo -Wcast-align -Wcast-qual -Woverloaded-virtual -Wredundant-decls -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wswitch-default -Wno-newline-eof -Wno-unused-function -Wno-unused-parameter)
    set(CMAKE_CXX_COMPILER  "/usr/bin/clang++")
endif ()

project(aoc-2024)
set(CMAKE_DEBUG_POSTFIX _dbg)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin") 
set(CMAKE_VERBOSE_MAKEFILE OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_available)

if(ipo_available AND (NOT CMAKE_BUILD_TYPE MATCHES Debug) AND (NOT CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    message("-- IPO enabled")
endif()

set(TARGETS day-01 day-02 day-03) # Add the other days as you please.

list(LENGTH TARGETS NUM_TARGETS)

foreach(current_target IN LISTS TARGETS)
    add_executable(${current_target} ${current_target}/${current_target}.cpp)
    set_target_properties(${current_target} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/aoclib/") 

    target_compile_options(${current_target} PRIVATE ${WARNING_FLAGS_CXX} $<$<CONFIG:Debug>:-fsanitize=undefined,address -g3 -Og>)
    target_link_options(${current_target} PRIVATE ${WARNING_FLAGS_CXX} $<$<CONFIG:Debug>:-fsanitize=undefined,address -g3 -Og>)

    # target_compile_definitions(${current_target} PRIVATE AOC_INPUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/input/${current_target}.txt")
    # target_compile_definitions(${current_target} PRIVATE AOC_INPUT_EXAMPLE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/input/${current_target}-example.txt")
    # target_compile_definitions(${current_target} PRIVATE AOC_INPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/input/")
    # target_compile_definitions(${current_target} PRIVATE AOC_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(${current_target} PRIVATE AOC_DAY_NAME="${current_target}")

    if(ipo_available AND (NOT CMAKE_BUILD_TYPE MATCHES Debug) AND (NOT CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
        set_property(TARGET ${current_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    add_custom_target("run-${current_target}"
        DEPENDS ${current_target}
        COMMAND ${current_target} ../input/${current_target}.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    )
    add_custom_target("run-${current_target}-example"
        DEPENDS ${current_target}
        COMMAND ${current_target} ../input/${current_target}-example.txt
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    )
endforeach(current_target)

# TODO...
add_custom_target("run-all"
    DEPENDS ${TARGETS}
    COMMAND day-01 ../input/day-01.txt
    COMMAND day-02 ../input/day-02.txt
    COMMAND day-03 ../input/day-03.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

add_custom_target("run-all-examples"
    DEPENDS ${TARGETS}
    COMMAND day-01 ../input/day-01-example.txt
    COMMAND day-02 ../input/day-02-example.txt
    COMMAND day-03 ../input/day-03-example.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
