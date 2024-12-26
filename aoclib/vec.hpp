#pragma once
#include <iostream>
#include <array>
#include "hash.hpp"

namespace aocutil 
{

template<typename T>
struct Vec2 
{
    using value_type = T;
    T x, y; 

    Vec2 operator+(const Vec2& v) const 
    {
        return Vec2{.x = x + v.x, .y = y + v.y};
    }

    Vec2& operator+=(const Vec2& v) 
    {
        x += v.x; 
        y += v.y;
        return *this;
    }

    Vec2 operator-(const Vec2& v) const 
    {
        return Vec2{.x = x - v.x, .y = y - v.y};
    }

    Vec2& operator-=(const Vec2& v) 
    {
        x -= v.x; 
        y -= v.y;
        return *this;
    }

    Vec2 operator-() const 
    {
        return Vec2{.x = -x, .y = -y};
    }

    Vec2 operator*(const T& scalar) const 
    {
        return Vec2{.x = x * scalar, .y = y * scalar};
    }

    Vec2 perp_dot(bool counterclockwise = true) const
    {
        // cos(90) -sin(90)  x
        // sin(90)  cos(90)  y
        // <=>
        //  0  -1  x
        //  1   0  y
        return counterclockwise ? Vec2{-y, x} : Vec2{y, -x}; 
    }

    friend Vec2 operator*(const T& scalar, const Vec2& v) 
    {
        return Vec2{.x = scalar * v.x, .y = scalar * v.y};
    }
    
    bool operator==(const Vec2& v) const = default;
};

enum class Direction {Up, Right, Down, Left, None};

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
constexpr std::array<Vec2<T>, 8> all_dirs_plus_diagonals_vec2() 
{
    constexpr Vec2<T> dir_right = {.x = 1, .y = 0};
    constexpr Vec2<T> dir_left = {.x = -1, .y = 0};
    constexpr Vec2<T> dir_up   = up_is_positive ? Vec2<T>{.x = 0, .y =  1} : Vec2<T>{.x = 0, .y = -1};
    constexpr Vec2<T> dir_down = up_is_positive ? Vec2<T>{.x = 0, .y = -1} : Vec2<T>{.x = 0, .y =  1};

    return {dir_right, dir_left, dir_up, dir_down,  dir_up + dir_right, dir_up + dir_left,  dir_down + dir_right, dir_down + dir_left};
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

template <typename T, bool up_is_positive = false>
constexpr Direction vec2_to_dir(const Vec2<T>& vec) 
{
    if (vec.y > 0 && vec.x == 0) {
        return up_is_positive ? Direction::Up : Direction::Down;
    } else if (vec.y < 0 && vec.x == 0) {
        return up_is_positive ? Direction::Down : Direction::Up;
    }

    if (vec.x != 0 && vec.y == 0) {
        return vec.x > 0 ? Direction::Right : Direction::Left;
    } 

    return Direction::None;
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