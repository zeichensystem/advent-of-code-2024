#include <stack>
#include <unordered_set>
#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/15
  
    Solutions: 
        - Part 1: 1465152 (Example: 10092)
        - Part 2: 1511259 (Example:  9021)
    Notes:  
        - Part 1: Fun.
        - Part 2: This got very ugly...
*/

using aocutil::Grid; 
using aocutil::Direction;
using aocutil::dir_to_vec2;
using Vec2 = aocutil::Vec2<int>;

void parse_input(const std::vector<std::string>& lines, Grid<char>& grid, std::vector<Direction>& robot_moves, bool part_two = false)
{
    bool in_moves = false;
    for (std::string line : lines) {
        aocio::str_remove_whitespace(line);
        if (part_two && !in_moves) {
            std::string new_line;
            for (char c : line) {
                if (c == 'O') {
                    new_line += "[]";
                } else if (c == '@') {
                    new_line += "@.";
                } else {
                    new_line += std::string(2, c);
                }
            }
            line = new_line;
        }

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

void robot_move_p2(Grid<char>& grid, Vec2& robot_pos, Direction move_dir)
{
    const Vec2 delta = dir_to_vec2<Vec2::value_type>(move_dir);

    if (move_dir == Direction::Left || move_dir == Direction::Right) {
        Vec2 end_pos = robot_pos + delta;
        for (auto sym = grid.try_get(end_pos); sym && (sym.value() == '[' || sym.value() == ']'); end_pos += delta, sym = grid.try_get(end_pos));
        if (grid.get(end_pos) != '.') {
            return;
        }
        for (Vec2 pos = end_pos; pos != robot_pos; pos -= delta) {
            grid.at(pos) = grid.at(pos - delta);
        }
        grid.at(robot_pos) = '.';
        robot_pos += delta;
        return;
    }

    struct Tile {
        Vec2 pos; 
        char sym;
        bool operator==(const Tile& t) const = default;
    };

    const auto get_tiles = [&grid = std::as_const(grid)](const Vec2& pos, const Vec2& delta) -> std::vector<Tile> {
        std::vector<Tile> tiles;
        Vec2 sym_pos = pos + delta;
        if (const auto sym = grid.try_get(sym_pos); sym.has_value()) {
            if (sym.value() == ']' || sym.value() == '[') {
                Vec2 sym2_delta = sym.value() == ']' ? Vec2{-1, 0} : Vec2{1, 0};
                Vec2 sym2_pos = sym_pos + sym2_delta;
                auto sym2 = grid.try_get(sym2_pos);
                assert(sym2.value() == ']' || sym2.value() == '[');
                if (sym2.value() == sym.value()) {
                    std::cout << sym_pos << "\n";
                    std::cout << grid;
                }
                assert(sym2.value() != sym.value());
                tiles.push_back(Tile{.pos = sym_pos,  .sym = sym.value()});
                tiles.push_back(Tile{.pos = sym2_pos, .sym = sym2.value()});
            } else if (sym.value() == '.') {
                tiles.push_back(Tile{.pos = pos + delta, .sym = '.'});
            }
        }
        return tiles; 
    };

    std::vector<Tile> end_tiles;
    std::unordered_set<Vec2> visited_box_tiles;
    std::stack<Tile> s;
    s.push(Tile{.pos=robot_pos, .sym = '@'});
    while (s.size()) { // Find end-tiles (depth-first).
        Tile t = s.top();
        if (t.sym != '.') {
            visited_box_tiles.insert(t.pos);
        }
        s.pop();
        if (t.sym == '.') {
            bool exists = false;
            for (Tile end_t : end_tiles) { // Hack...
                if (end_t == t) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                end_tiles.push_back(t);
            }
            continue;
        }
        const std::vector<Tile> next_tiles = get_tiles(t.pos, delta);
        if (next_tiles.size() == 0) { // No moves.
            return;
        }
        for (Tile nt : next_tiles) {
            s.push(nt);
        }
    }

    for (Tile start_tile : end_tiles) { // Move previous tiles upwards/downwards to the end-tiles (depth-first).
        std::stack<Tile> s2; 
        s2.push(start_tile);
        while (s2.size()) {
            Tile t = s2.top();
            s2.pop();
            Vec2 prev_pos = t.pos - delta;
            if (!visited_box_tiles.contains(prev_pos)) {
                continue;
            }
            assert(t.sym == '.');
            if (auto prev_sym = grid.try_get(prev_pos); prev_sym.has_value() && (prev_sym.value() == '[' || prev_sym.value() == ']' || prev_sym.value() == '@')) {
                if (prev_sym.value() == '@') {
                    grid.at(t.pos) = '@';
                    grid.at(prev_pos) = '.';
                    continue;
                } else if (prev_pos.y == robot_pos.y) {
                    continue;
                }
                grid.at(t.pos) = prev_sym.value();
                grid.at(prev_pos) = '.';
                s2.push(Tile{.pos = prev_pos, .sym = '.'});
            }
        }
    }
    robot_pos += delta;
    assert(grid.get(robot_pos) == '@');
}

auto part_one(const std::vector<std::string>& lines, bool part_two = false)
{
    Grid<char> grid; 
    std::vector<Direction> robot_moves; 
    parse_input(lines, grid, robot_moves, part_two);
    Vec2 robot_pos = grid.find_elem_positions_if([](char c) { return c == '@'; }).at(0);

    for (Direction move_dir : robot_moves) {
        !part_two ? robot_move(grid, robot_pos, move_dir) : robot_move_p2(grid, robot_pos, move_dir);
    }

    int gps_sum = 0;
    grid.foreach([&gps_sum, part_two](const Vec2& pos, char sym) {
        if ((!part_two && sym == 'O') || (part_two && sym == '[')) {
            gps_sum += pos.x + pos.y * 100;
        }
    });

    return gps_sum;
}

auto part_two(const std::vector<std::string>& lines)
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