#include <numeric>
#include <limits>
#include <unordered_map>
#include "aoclib/aocio.hpp"
#include "aoclib/hash.hpp"

/*
    Problem: https://adventofcode.com/2024/day/11
  
    Solutions: 
        - Part 1:          172484 (Example:          55312)
        - Part 2: 205913561055242 (Example: 65601038650482)
    Notes:  
        - Part 1: Dumb solution without caching for Part 1 ('apply_rules()')
        - Part 2: Smarter solution with caching for Part 2 ('len_after_blinks()')
*/

int log10_i64(int64_t n)
{
    assert(n > 0);
    const int MAX_EXP_i64 = 18; // == std::floorf(std::log10f(std::numeric_limits<int64_t>::max()));
    int exp = 0; 
    int64_t pow_of_10 = 1; 
    while (exp < MAX_EXP_i64 && pow_of_10 * 10 <= n) {
        pow_of_10 *= 10;
        ++exp;
    }
    return exp;
}

int64_t pow10_i64(int exp)
{
    assert(exp != 0);
    if (exp < 0) {
        return 0; 
    }
    const int MAX_EXP_i64 = 18; // == std::floorf(std::log10f(std::numeric_limits<int64_t>::max()));
    int64_t pow_of_10 = 1; 
    for (int i = 0; i < (exp < MAX_EXP_i64 ? exp : MAX_EXP_i64); ++i) {
        pow_of_10 *= 10; 
    }
    return pow_of_10;
}

std::vector<int64_t> apply_rules(const std::vector<int64_t>& input, int num_blinks = 1)
{
    std::vector<int64_t> in = input; 
    std::vector<int64_t> out;
    for (int i = 0; i < num_blinks; ++i) {
        for (int64_t stone : in) {
            if (stone == 0) {
                out.push_back(1);
            } 
            else if (const int n_digits = log10_i64(stone) + 1; n_digits % 2 == 0) {
                int64_t div = pow10_i64(n_digits / 2);
                assert(div > 0);
                int64_t left_half = stone / div; 
                int64_t right_half = stone % div; 
                out.push_back(left_half); 
                out.push_back(right_half);
            } 
            else {
                out.push_back(stone * 2024);
            }
        }
        if (i < num_blinks - 1) {
            std::swap(out, in);
            out = {};
        }
    }
    return out;
}

template <>
struct std::hash<std::pair<int64_t, int>>
{
  std::size_t operator()(const std::pair<int64_t, int>& pair) const 
  {
    std::size_t hash = 0; 
    aocutil::hash_combine(hash, pair.first);
    aocutil::hash_combine(hash, pair.second);
    return hash;
  }
};

int64_t len_after_blinks(int64_t stone, int num_blinks, std::unordered_map<std::pair<int64_t, int>, int64_t>& cache)
{
    if (num_blinks == 0) {
        return 1;
    }

    if (cache.contains(std::make_pair(stone, num_blinks))) {
        return cache.at(std::make_pair(stone, num_blinks));
    }

    if (stone == 0) {
        return len_after_blinks(1, num_blinks - 1, cache); 
    } else if (const int n_digits = log10_i64(stone) + 1; n_digits % 2 == 0) {
        int div = pow10_i64(n_digits / 2); 
        int stone_left = stone / div; 
        int stone_right = stone % div;
        int64_t result = len_after_blinks(stone_left, num_blinks - 1, cache) + len_after_blinks(stone_right, num_blinks - 1, cache);
        cache.insert({std::make_pair(stone, num_blinks), result});
        return result;
    } else {
        int64_t result = len_after_blinks(stone * 2024, num_blinks - 1, cache);
        cache.insert({std::make_pair(stone, num_blinks), result});
        return result;
    }
}

int64_t part_one(const std::vector<std::string>& lines)
{   
    const std::vector<int64_t> stones = aocio::line_tokenise(lines.at(0), " ", "", [](const std::string& s) -> int64_t { return aocio::parse_num_i64(s).value(); });
    return apply_rules(stones, 25).size();
}

int64_t part_two(const std::vector<std::string>& lines)
{
    const std::vector<int64_t> stones = aocio::line_tokenise(lines.at(0), " ", "", [](const std::string& s) -> int64_t { return aocio::parse_num_i64(s).value(); });
    std::unordered_map<std::pair<int64_t, int>, int64_t> cache; // {stone_x, num_blinks_y} -> len_after_blinks(stone_x, num_blinks_y)

    return std::transform_reduce(stones.cbegin(), stones.cend(), int64_t{0}, std::plus{}, [&cache](int64_t stone) {
        return len_after_blinks(stone, 75, cache);
    });
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