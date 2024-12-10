#include <stack>
#include "../aoclib/aocio.hpp"
#include "../aoclib/grid.hpp"
#include "../aoclib/vec.hpp"

/*
    Problem: https://adventofcode.com/2024/day/10
  
    Solutions: 
        - Part 1: 557 (Example: 36)
        - Part 2: 1062 (Example: 81)
    Notes:  
        - Part 1: Simple depth-first search (but if a '9'-destination can be reached in multiple paths from 
                  one trailhead, make sure to only count one of them (using the "reached" grid) since the puzzle 
                  description says only the longest path matters). 
        - Part 2: Exactly as above, but without the "reached" grid (i.e. we count all possible paths). Kind of 
                  funny, since that's what I did initally for Part 1 because I thought I had to count all paths :)
*/

using aocutil::Grid; 
using Vec2 = aocutil::Vec2<int>;

int part_one(const std::vector<std::string>& lines, bool part_two = false)
{
     Grid<int> height_map; 
     for (const auto& line : lines) {
        std::vector<int> row; 
        std::transform(line.cbegin(), line.cend(), std::back_inserter(row), [](char c) {
            return aocio::parse_digit(c).value();
        });
        height_map.push_row(row);
     }

    const auto get_adjacent = [&height_map = std::as_const(height_map)](const Vec2& pos) {
        const auto dirs = aocutil::all_dirs_vec2<int>(); 
        std::vector<Vec2> neighbors; 
        for (const auto dir : dirs) {
            if (height_map.try_get(pos + dir).has_value()) {
                neighbors.push_back(pos + dir);
            }
        }
        return neighbors; 
    };

    int result = 0; 
    const std::vector<Vec2> trailheads =  height_map.find_elem_positions_if([](int h) {return h == 0;}); 
    for (const auto& trailhead : trailheads) {
        int score = 0; 
        std::stack<Vec2> positions; 
        positions.push(trailhead);
        Grid<bool> reached(height_map.width(), height_map.height(), false);

        while (!positions.empty()) { // Depth-first search.
            const Vec2 pos = positions.top(); 
            positions.pop();
            const int current_height = height_map.get(pos);
            std::vector<Vec2> adjacent = get_adjacent(pos); 
            for (const auto& adj_pos : adjacent) {
                int neighbor_height = height_map.get(adj_pos); 
                if (neighbor_height == current_height + 1) {
                    if (neighbor_height == 9 && !reached.get(adj_pos)) {
                        reached.set(adj_pos, !part_two); // Only use the "reached" grid for Part 1 (i.e. count all paths for Part 2)
                        ++score;
                    } else {
                        positions.push(adj_pos); 
                    }
                }
            }
        }
        result += score; 
    }
    return result;
}

int part_two(const std::vector<std::string>& lines)
{
    return part_one(lines, true);
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
        auto p1 = part_one(lines);  
        std::cout << "- Part 1: " << p1 << "\n";
        auto p2 = part_two(lines);
        std::cout << "- Part 2: " << p2 << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << "Guru Meditation: " << err.what() << "\n";
        return EXIT_FAILURE;
    }
}