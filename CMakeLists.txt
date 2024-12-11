# Within directory build/Release or build/Debug: Select debug/release variant with % cmake -DCMAKE_BUILD_TYPE=Debug/Release ../..
# Build and run: make run-day-XX or  cmake --build . --target run-day-nn

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT MSVC)
    set(WARNING_FLAGS_CXX -Wall -Wextra -Wpedantic -Wvla -Wshadow -Wundef -Wmisleading-indentation -Wnull-dereference -Wshadow  -Wundef -Wstrict-overflow=5 -Wsign-promo -Wcast-align -Wcast-qual -Woverloaded-virtual -Wredundant-decls -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wswitch-default -Wno-newline-eof -Wno-unused-function -Wno-unused-parameter)
    set(DBG_FLAGS_CXX -fsanitize=undefined,address -g3 -Og)
else ()
    set(WARNING_FLAGS_CXX /W4)
    set(DBG_FLAGS_CXX /fsanitize=address)
endif ()

project(aoc-2024)
set(CMAKE_DEBUG_POSTFIX _dbg)

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) 
endif()

# set(CMAKE_VERBOSE_MAKEFILE OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_available)

if (ipo_available AND (NOT CMAKE_BUILD_TYPE MATCHES Debug) AND (NOT CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    message("-- IPO enabled")
endif()

set(TARGETS day-01 day-02 day-03 day-04 day-05 day-06 day-07 day-08 day-09 day-10) # Add the other days as you please.

list(LENGTH TARGETS NUM_TARGETS)

foreach(current_target IN LISTS TARGETS)
    add_executable(${current_target} ${current_target}/${current_target}.cpp)
    set_target_properties(${current_target} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
    target_include_directories(${current_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/") 

    target_compile_options(${current_target} PRIVATE ${WARNING_FLAGS_CXX} $<$<CONFIG:Debug>:${DBG_FLAGS_CXX}>)
    target_link_options(${current_target} PRIVATE ${WARNING_FLAGS_CXX} $<$<CONFIG:Debug>:${DBG_FLAGS_CXX}>)

    target_compile_definitions(${current_target} PRIVATE AOC_DAY_NAME="${current_target}")

    if(ipo_available AND (NOT CMAKE_BUILD_TYPE MATCHES Debug) AND (NOT CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
        set_property(TARGET ${current_target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    
    add_custom_target("run-${current_target}"
        DEPENDS ${current_target}
        COMMAND ${current_target} ${CMAKE_CURRENT_SOURCE_DIR}/input/${current_target}.txt
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
    add_custom_target("run-${current_target}-example"
        DEPENDS ${current_target}
        COMMAND ${current_target} ${CMAKE_CURRENT_SOURCE_DIR}/input/${current_target}-example.txt
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endforeach(current_target)

# TODO...
add_custom_target("run-all"
    DEPENDS ${TARGETS}
    COMMAND day-01 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-01.txt
    COMMAND day-02 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-02.txt
    COMMAND day-03 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-03.txt
    COMMAND day-04 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-04.txt
    COMMAND day-05 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-05.txt
    COMMAND day-06 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-06.txt
    COMMAND day-07 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-07.txt
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_custom_target("run-all-examples"
    DEPENDS ${TARGETS}
    COMMAND day-01 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-01-example.txt
    COMMAND day-02 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-02-example.txt
    COMMAND day-03 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-03-example.txt
    COMMAND day-04 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-04-example.txt
    COMMAND day-05 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-05-example.txt
    COMMAND day-06 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-06-example.txt
    COMMAND day-07 ${CMAKE_CURRENT_SOURCE_DIR}/input/day-07-example.txt
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
