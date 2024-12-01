#pragma once

#include <memory>

/* 
    hash_combine copied verbatim from https://stackoverflow.com/a/57595105 (last retrieved 2024-06-20)
    which was posted by user https://stackoverflow.com/users/387023/j00hi
*/

namespace aocutil 
{
template <typename T, typename... Rest>
static inline void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}
}