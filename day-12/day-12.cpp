#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"
#include <stack>

/*
    Problem: https://adventofcode.com/2024/day/12
  
    Solutions: 
        - Part 1: 1415378 (Example: 1930)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

using aocutil::Grid; 
using Vec2 = aocutil::Vec2<int>;

struct Plot {
    Vec2 pos;
    int num_fences = 4; 
};

int region_price(const Grid<char>& grid, Grid<int>& visited, const Vec2& pos)
{
    const char plant_sym = grid.at(pos);
    std::vector<Plot> plots; 

    std::stack<Vec2> s; 
    s.push(pos); 
    while (!s.empty()) { // DFS flood-fill.
        const Vec2 cur_pos = s.top(); 
        s.pop(); 
        if (visited.at(cur_pos) != 0) {
            continue;
        }
        visited.at(cur_pos) = 1;
        plots.push_back(Plot{.pos = cur_pos, .num_fences = 4});

        int num_adjacent = 0;
        constexpr std::array<Vec2, 4> dirs = aocutil::all_dirs_vec2<int>();
        for (const Vec2 dir: dirs) { // For all adjacent plots. 
            const Vec2 adj_pos = cur_pos + dir; 
            if (auto adj_sym = grid.try_get(adj_pos); adj_sym.has_value() && adj_sym.value() == plant_sym) {
                ++num_adjacent;
                if (!visited.at(adj_pos)) {
                    s.push(adj_pos);
                }
            }
        }
        assert(num_adjacent <= 4);
        plots.back().num_fences = 4 - num_adjacent;
    }

    int area = 0, perimeter = 0; 
    for (const Plot& plot : plots) {
        area += 1; 
        perimeter += plot.num_fences;
    }

    return area * perimeter;
}

int part_one(const std::vector<std::string>& lines)
{
    const Grid<char> garden{lines}; 
    Grid<int> visited(garden.width(), garden.height(), 0);

    int result = 0; 
    garden.foreach([&garden, &visited, &result](const Vec2& pos, char elem) {
        if (visited.at(pos) != 0) {
            return;
        } else {
            result += region_price(garden, visited, pos); 
        }
    });

    return result;
}

int part_two(const std::vector<std::string>& lines)
{
    return -1; 
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