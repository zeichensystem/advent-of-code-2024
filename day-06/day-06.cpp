#include "../aoclib/aocio.hpp"
#include "../aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/6
  
    Solutions: 
        - Part 1: 5080 (Example: 41)
        - Part 2: 1919 (Example:  6)
    Notes:  
        - Part 1: 
        - Part 2: Brute-force solution which was really slow (~42 seconds in Release mode), but got reasonably fast (~0.3 seconds in Release, ~9.5 seconds in Debug with ASAN etc.) 
                  after I implemented Optimisation #1 and #2 below.
*/

using Vec2 = aocutil::Vec2<int>;
using aocutil::Grid;
using aocutil::Direction;

constexpr uint8_t DIR_NONE = 0, DIR_UP = 1, DIR_DOWN = 2, DIR_LEFT = 4, DIR_RIGHT = 8;

constexpr uint8_t dir_to_flag(Direction dir)
{
    switch (dir)
        {
        case Direction::Up:
            return DIR_UP;
        case Direction::Down:
            return DIR_DOWN;
        case Direction::Right:
            return DIR_RIGHT;
        case Direction::Left:
            return DIR_LEFT;
        default:
            return DIR_NONE;
        }
}

bool guard_wander(const Grid<char>& grid, Grid<uint8_t>& visited_grid, const Vec2& start_pos)
{
    for (auto& dir_flag : visited_grid) {
        dir_flag = DIR_NONE;
    }
    
    Vec2 guard_pos = start_pos;
    Direction guard_dir = Direction::Up;

    while (grid.try_get(guard_pos).has_value()) {
        
        if (!visited_grid.at(guard_pos)) {
            visited_grid.at(guard_pos) = dir_to_flag(guard_dir);
        } else if (visited_grid.at(guard_pos) & dir_to_flag(guard_dir)) { // Field was already visited coming from the current direction:
            return false; // -> Guard got caught in a loop.
        } else {
            visited_grid.at(guard_pos) |= dir_to_flag(guard_dir);
        }

        Vec2 delta = aocutil::dir_to_vec2<int>(guard_dir);
        int turns = 0; 
        while (grid.try_get(guard_pos + delta).has_value() && grid.try_get(guard_pos + delta).value() == '#' && turns < 4) {
            guard_dir = aocutil::dir_get_left_right(guard_dir).second; // Turn right.
            delta = aocutil::dir_to_vec2<int>(guard_dir);
            ++turns; 
        }
        if (turns == 4) {
            throw std::invalid_argument("guard_wander: The guard seems to be stuck inside 4 walls...");
        }
        guard_pos = guard_pos + delta; 
    }

    return true; // Guard did not get caught in a loop.
}

int part_one(const std::vector<std::string>& lines)
{
    const Grid<char> grid {lines}; 
    const std::vector<Vec2> start_pos = grid.find_elem_positions('^');
    if (start_pos.size() != 1) {
        throw std::invalid_argument("part_one: No (or more than one) guard.");
    }

    Grid<uint8_t> visited_grid; 
    for (int i = 0; i < grid.height(); ++i) {
        visited_grid.push_row(std::vector<uint8_t>(grid.width(), DIR_NONE));
    }

    guard_wander(grid, visited_grid, start_pos.front());
    return std::count_if(visited_grid.cbegin(), visited_grid.cend(), [](uint8_t flag) -> bool { return flag != DIR_NONE; });
}

int part_two(const std::vector<std::string>& lines)
{
    Grid<char> grid {lines}; 
    const std::vector<Vec2> start_pos = grid.find_elem_positions('^');
    if (start_pos.size() != 1) {
        throw std::invalid_argument("part_two: No (or more than one) guard.");
    }

    // Optimisation #1: Use a grid of uint8_t flags instead of an unordered_map<Vec2, unordered_set<Direction>>.
    Grid<uint8_t> visited_grid;
    for (int i = 0; i < grid.height(); ++i) {
        visited_grid.push_row(std::vector<uint8_t>(grid.width(), DIR_NONE));
    }

    // Optimisation #2: We only need to consider potential obstacles in the initial path of the guard. 
    guard_wander(grid, visited_grid, start_pos.front());
    const std::vector<Vec2> candidates = visited_grid.find_elem_positions_if([](uint8_t flag) -> bool { return flag != DIR_NONE; });

    int num_obstructions = 0; 
    for (const Vec2& obstruction_pos : candidates) {
        if (obstruction_pos == start_pos.front()) { // Don't drop obstacles on the guard...
            continue;
        }
        grid.set(obstruction_pos, '#');
        num_obstructions += guard_wander(grid, visited_grid, start_pos.front()) ? 0 : 1;
        grid.set(obstruction_pos, '.');
    }
    return num_obstructions;
}

int main(int argc, char* argv[])
{
    aocio::print_day();

    std::vector<std::string> lines;
    const aocio::IOStatus status = aocio::handle_input(argc, argv, lines);

    if (!aocio::iostat_has_flag(status, aocio::IOStatus::INPUT_SUCCESS)) {
        return EXIT_FAILURE;
    } else if (aocio::iostat_has_flag(status, aocio::IOStatus::INPUT_HELP)) {
        return EXIT_SUCCESS; 
    }

    try {
        int p1 = part_one(lines);  
        std::cout << "- Part 1: " << p1 << "\n";
        int p2 = part_two(lines);
        std::cout << "- Part 2: " << p2 << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << "Guru Meditation: " << err.what() << "\n";
        return EXIT_FAILURE;
    }
}