#include <unordered_map>
#include <unordered_set>
#include "../aoclib/aocio.hpp"
#include "../aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/8
  
    Solutions: 
        - Part 1:  295 (Example: 14)
        - Part 2: 1034 (Example: 34)
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;
using aocutil::Grid;

int part_one(const std::vector<std::string>& lines, bool is_part_two = false)
{
    const Grid<char> grid {lines};
    std::unordered_map<char, std::vector<Vec2>> antenna_positions; // {antenna_type_xy -> [pos_1, ..., pos_n], ...}
    std::unordered_set<Vec2> antinode_positions;

    grid.foreach([&antenna_positions](const Vec2& pos, char elem) {
        if (elem == '.') {
            return;
        }
        if (antenna_positions.contains(elem)) {
            antenna_positions.at(elem).push_back(pos);
        } else {
            antenna_positions.insert({elem, std::vector<Vec2>{pos}});
        }
    });
    
    const auto handle_pair_p1 = [&grid, &antinode_positions](const Vec2& a, const Vec2& b) {
        const Vec2 behind_a = b + 2 * (a - b), 
                   behind_b = a + 2 * (b - a);
        for (const Vec2& candidate : {behind_a, behind_b}) {
            if (grid.pos_on_grid(candidate)) {
                antinode_positions.insert(candidate);
            }
        }
    };
    const auto handle_pair_p2 = [&grid, &antinode_positions](const Vec2& a, const Vec2& b) {
        for (int dir : {1, -1}) {
            Vec2 candidate = a; 
            int n = 1;
            while (grid.pos_on_grid(candidate)) {
                antinode_positions.insert(candidate);
                candidate = a + (n++ * dir) * (a - b);
            }
        }
    };

    for (auto const& [antenna_type, positions] : antenna_positions) {
        const int last_i = positions.size() ? std::size(positions) - 1 : 0;
        for (int i = 0; i < last_i; ++i) {
            const Vec2 a = positions.at(i);
            for (int j = i + 1; j < std::ssize(positions); ++j) {
                const Vec2 b = positions.at(j);
                !is_part_two ? handle_pair_p1(a, b) : handle_pair_p2(a, b);
            }
        }
    }

    return antinode_positions.size();
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