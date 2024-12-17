#include "aoclib/aocio.hpp"
#include "aoclib/vec.hpp"

/*
    Problem: https://adventofcode.com/2024/day/14
  
    Solutions: 
        - Part 1: 231019008 (Example with grid = {11, 7}: 12)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;

struct Robot {
    Vec2 pos, vel;
};

struct RobotParser : public aocio::RDParser 
{
    RobotParser(const std::vector<std::string>& lines) : aocio::RDParser(lines, "=,- \t", "=,-", false) {};

    std::vector<Robot> parse()
    {
        std::vector<Robot> robots;
        while (!is_end()) {
            Robot bot; 
            bot.pos = point();
            bot.vel = velocity();
            robots.push_back(bot);
        }
        return robots;
    }

    Vec2 point()
    {
        Vec2 p;
        require_token("p");
        require_token("=");
        p.x = require_int<Vec2::value_type>();
        require_token(",");
        p.y = require_int<Vec2::value_type>();
        return p;
    }

    Vec2 velocity()
    {
        Vec2 v;
        require_token("v");
        require_token("=");
        Vec2::value_type sign = 1;
        if (accept_token("-")) {
            sign = -1;
        }
        v.x = require_int<Vec2::value_type>() * sign;
        require_token(",");
        sign = 1;
        if (accept_token("-")) {
            sign = -1;
        }
        v.y = require_int<Vec2::value_type>() * sign;
        return v;
    }
};

Vec2 simulate_robot(const Robot& bot, const Vec2& grid, int seconds)
{
    Vec2 new_pos;  
    new_pos.x = (bot.pos.x + bot.vel.x * seconds) % grid.x;
    new_pos.y = (bot.pos.y + bot.vel.y * seconds) % grid.y;
    if (new_pos.x < 0) {
        new_pos.x += grid.x;
    }
    if (new_pos.y < 0) {
        new_pos.y += grid.y;
    }
    return new_pos;

}

int part_one(const std::vector<std::string>& lines)
{
    constexpr int elapsed_seconds = 100; 
    constexpr Vec2 grid = {101, 103}; // Example: {11, 7}

    std::vector<Robot> robots = RobotParser(lines).parse();

    std::vector<Vec2> end_positions;
    std::transform(robots.cbegin(), robots.cend(), std::back_inserter(end_positions), [grid](const Robot& bot) {
        return simulate_robot(bot, grid, elapsed_seconds);
    });

    std::array<int, 4> quadrant_cnt = {0,0,0,0};
    for (const Vec2& pos: end_positions) {
        if (pos.x < grid.x / 2) { // Left
            if (pos.y < grid.y / 2) { // Left-Top
                quadrant_cnt.at(0) += 1;
            } else if (pos.y > grid.y / 2) { // Left-Bottom
                quadrant_cnt.at(1) += 1;
            }
        } 
        else if (pos.x > grid.x / 2) { // Right
            if (pos.y < grid.y / 2) { // Right-Top
                quadrant_cnt.at(2) += 1;
            } else if (pos.y > grid.y / 2) { // Right-Bottom
                quadrant_cnt.at(3) += 1;
            }
        } 
    }
    
    int safety_factor = 1; 
    for (int cnt : quadrant_cnt) {
        safety_factor *= cnt;
    }
    return safety_factor;
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