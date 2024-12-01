#pragma once

#include <array>
#include "hash.hpp"

namespace aocutil 
{

template<typename T>
struct Vec2 
{
    T x, y; 

    Vec2 operator+(const Vec2& v) const 
    {
        return Vec2{.x = x + v.x, .y = y + v.y};
    }

    Vec2 operator-(const Vec2& v) const 
    {
        return Vec2{.x = x - v.x, .y = y - v.y};
    }
    
    bool operator==(const Vec2& v) const = default;
};

enum class Direction {Up, Right, Down, Left};

template <typename T, bool up_is_positive = false>
constexpr std::array<Vec2<T>, 4> all_dirs_vec2() 
{
    constexpr Vec2<T> dir_right = {.x = 1, .y = 0};
    constexpr Vec2<T> dir_left = {.x = -1, .y = 0};
    constexpr Vec2<T> dir_up   = up_is_positive ? Vec2<T>{.x = 0, .y =  1} : Vec2<T>{.x = 0, .y = -1};
    constexpr Vec2<T> dir_down = up_is_positive ? Vec2<T>{.x = 0, .y = -1} : Vec2<T>{.x = 0, .y =  1};

    return {dir_right, dir_left, dir_up, dir_down};
}

template <typename T, bool up_is_positive = false>
constexpr Vec2<T> dir_to_vec2(Direction dir) 
{
    constexpr Vec2<T> dir_right = {.x = 1, .y = 0};
    constexpr Vec2<T> dir_left = {.x = -1, .y = 0};
    constexpr Vec2<T> dir_up   = up_is_positive ? Vec2<T>{.x = 0, .y =  1} : Vec2<T>{.x = 0, .y = -1};
    constexpr Vec2<T> dir_down = up_is_positive ? Vec2<T>{.x = 0, .y = -1} : Vec2<T>{.x = 0, .y =  1};

    switch (dir)
    {
    case Direction::Up:
        return dir_up;

    case Direction::Right: 
        return dir_right; 

    case Direction::Down:
        return dir_down; 

    case Direction::Left:
        return dir_left;     

    default:
        throw std::invalid_argument("dir_to_vec2: Invalid direction");
        break;
    }
}

constexpr std::pair<Direction, Direction> dir_get_left_right(Direction dir)
{
    switch (dir)
    {
    case Direction::Right:
        return {Direction::Up, Direction::Down}; 
    case Direction::Left:   
        return {Direction::Down, Direction::Up}; 

    case Direction::Up:
        return {Direction::Left, Direction::Right};
    case Direction::Down:
         return {Direction::Right, Direction::Left};

    default:
        throw std::invalid_argument("dir_get_left_right: Invalid direction");
    }
}

}

template<typename T>
struct std::hash<aocutil::Vec2<T>>
{
    std::size_t operator()(const aocutil::Vec2<T>& v) const noexcept
    {
        std::size_t h = 0;
        aocutil::hash_combine(h, v.x, v.y);
        return h; 
    }
};

template<typename T>
inline std::ostream& operator<<(std::ostream&os, const aocutil::Vec2<T>& v) {
    return os << "(x: " << v.x << ", y: " << v.y << ")";
}