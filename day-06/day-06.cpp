#include <unordered_map>
#include <unordered_set>
#include "../aoclib/aocio.hpp"
#include "../aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/6
  
    Solutions: 
        - Part 1: 5080 (Example: 41)
        - Part 2: 1919 (Example:  6)
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;
using aocutil::Grid;
using aocutil::Direction;

int part_one(const std::vector<std::string>& lines)
{
    Grid<char> grid {lines}; 
    const std::vector<Vec2> start_pos = grid.find_elem_positions('^');
    if (start_pos.size() != 1) {
        throw "part_one: No (or more than one) guards.";
    }
    
    Vec2 guard_pos = start_pos.at(0);
    Direction guard_dir = Direction::Up;
    while (grid.try_get(guard_pos).has_value()) {
        grid.set(guard_pos, 'X');
        Vec2 delta = aocutil::dir_to_vec2<int>(guard_dir);
        int turns = 0; 
        while (grid.try_get(guard_pos + delta).has_value() && grid.try_get(guard_pos + delta).value() == '#' && turns < 4) {
            guard_dir = aocutil::dir_get_left_right(guard_dir).second; // Turn right.
            delta = aocutil::dir_to_vec2<int>(guard_dir);
            ++turns; 
        }
        if (turns == 4) {
            throw "part_one: Guard is stuck.";
        }
        guard_pos = guard_pos + delta; 
    }

    return grid.find_elem_positions('X').size();
}

int part_two(const std::vector<std::string>& lines)
{
    Grid<char> grid {lines}; 
    const std::vector<Vec2> start_pos = grid.find_elem_positions('^');
    if (start_pos.size() != 1) {
        throw "part_one: No (or more than one) guards.";
    }

    const std::vector<Vec2> candidates = grid.find_elem_positions('.');
    int num_obstructions = 0; 

    for (const Vec2& obstruction_pos : candidates) {
        grid.set(obstruction_pos, '#');

        Vec2 guard_pos = start_pos.at(0);
        Direction guard_dir = Direction::Up;
        std::unordered_map<Vec2, std::unordered_set<Direction>> visited;
        bool loop = false; 

        while (grid.try_get(guard_pos).has_value()) {
            if (!visited.contains(guard_pos)) {
                visited.insert({guard_pos, std::unordered_set<Direction>{guard_dir}});
            } else if (visited.at(guard_pos).contains(guard_dir)) {
                loop = true; 
                break;
            } else {
                visited.at(guard_pos).insert(guard_dir);
            }
            
            Vec2 delta = aocutil::dir_to_vec2<int>(guard_dir);
            int turns = 0; 
            while (grid.try_get(guard_pos + delta).has_value() && grid.try_get(guard_pos + delta).value() == '#' && turns < 4) {
                guard_dir = aocutil::dir_get_left_right(guard_dir).second; // Turn right.
                delta = aocutil::dir_to_vec2<int>(guard_dir);
                ++turns; 
            }
            if (turns == 4) {
                throw "part_one: Guard is stuck.";
            }
            guard_pos = guard_pos + delta; 
        }

        if (loop) {
            ++num_obstructions;
        }

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