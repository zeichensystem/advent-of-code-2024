#include <numeric>
#include "../aoclib/aocio.hpp"
#include "../aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/4
  
    Solutions: 
        - Part 1: 2414 (Example: 18)
        - Part 2: 1871 (Example:  9)
    Notes:  
        - Part 1: 
        - Part 2:
*/

using aocutil::Grid;
using Vec2 = aocutil::Vec2<int>;

int part_one(const std::vector<std::string>& lines, bool part_two = false)
{
    Grid<char> grid{lines}; 
    std::vector<Vec2> candidates = grid.find_elem_positions(part_two ? 'A' : 'X');

    const auto match_xmas = [&grid](const Vec2& pos) -> int { // For Part 1. 
        const std::string XMAS_STR = "XMAS";
        std::array<int, 8> direction_matched = {0, 0, 0, 0, 0, 0, 0, 0}; 

        for (int d = 0; d < std::ssize(XMAS_STR); ++d) {
            const Vec2 d_up = {0, -d}, d_down = {0, d}, d_right = {d, 0}, d_left = {-d, 0};
            const std::array<Vec2, 8> direction_deltas = {d_up, d_down, d_right, d_left, d_up + d_right, d_up + d_left, d_down + d_right, d_down + d_left};
            for (size_t i = 0; i < direction_deltas.size(); ++i) {
                if (const auto ch = grid.try_get(pos + direction_deltas.at(i)); ch.has_value() && ch.value() == XMAS_STR.at(d)) {
                    direction_matched.at(i) += 1;
                } 
            }
        }
        return std::count_if(direction_matched.cbegin(), direction_matched.cend(), [&XMAS_STR](int n) {return n == std::ssize(XMAS_STR);});
    };

    const auto match_cross_mas = [&grid](const Vec2& pos) -> int { // For Part 2. 
        const std::array<std::pair<Vec2, Vec2>, 2> cross_dirs = {std::make_pair(Vec2{1, -1}, Vec2{-1, 1}), std::make_pair(Vec2{-1, -1}, Vec2{1, 1})};
        for (const auto& dir_pair : cross_dirs) {
            const auto ch_top = grid.try_get(pos + dir_pair.first); 
            const auto ch_bottom = grid.try_get(pos + dir_pair.second);
            if (!ch_top.has_value() || !ch_bottom.has_value()) {
                return 0; 
            }
            const bool mas_cross = (ch_top.value() == 'M' && ch_bottom.value() == 'S') || (ch_top.value() == 'S' && ch_bottom.value() == 'M');
            if (!mas_cross) {
                return 0;
            }
        }
        return 1;
    };

    int total_matches = 0; 
    for (const Vec2& pos : candidates) {
        total_matches += part_two ? match_cross_mas(pos) : match_xmas(pos);
    }
    return total_matches;
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