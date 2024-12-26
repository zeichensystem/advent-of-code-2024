#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/15
  
    Solutions: 
        - Part 1: 1465152 (Example: 10092)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

using aocutil::Grid; 
using aocutil::Direction;
using aocutil::dir_to_vec2;
using Vec2 = aocutil::Vec2<int>;

void parse_input(const std::vector<std::string>& lines, Grid<char>& grid, std::vector<Direction>& robot_moves)
{
    bool in_moves = false;
    for (std::string line : lines) {
        aocio::str_remove_whitespace(line);
        if (line.size() == 0) {
            in_moves = true; 
            continue;
        }
        if (!in_moves) {
            grid.push_row(line);
        } else {
            for (char c : line) {
                switch (c) {
                    case '<':
                        robot_moves.push_back(Direction::Left);
                        break;
                    case '>':
                        robot_moves.push_back(Direction::Right);
                        break;
                    case '^':
                        robot_moves.push_back(Direction::Up);
                        break;
                    case 'v':
                        robot_moves.push_back(Direction::Down);
                        break;
                    default:
                        throw std::runtime_error("parse: Invalid direction symbol");
                    }
            }
        }  
    }
}

void robot_move(Grid<char>& grid, Vec2& robot_pos, Direction move_dir)
{
    const Vec2 delta = dir_to_vec2<Vec2::value_type>(move_dir);

    Vec2 end_pos = robot_pos + delta;
    for (auto sym = grid.try_get(end_pos); sym && sym.value() == 'O'; end_pos += delta, sym = grid.try_get(end_pos));
    
    if (grid.get(end_pos) != '.') {
        return;
    }

    for (Vec2 pos = end_pos; pos != robot_pos; pos -= delta) {
        grid.at(pos) = grid.at(pos - delta);
    }
    grid.at(robot_pos) = '.';
    robot_pos += delta;
}

auto part_one(const std::vector<std::string>& lines)
{
    Grid<char> grid; 
    std::vector<Direction> robot_moves; 
    parse_input(lines, grid, robot_moves);
    Vec2 robot_pos = grid.find_elem_positions_if([](char c) { return c == '@'; }).at(0);

    for (Direction move_dir : robot_moves) {
        robot_move(grid, robot_pos, move_dir);
    }
    
    int gps_sum = 0;
    grid.foreach([&gps_sum](const Vec2& pos, char sym) {
        if (sym == 'O') {
            gps_sum += pos.x + pos.y * 100;
        }
    });
    return gps_sum;
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