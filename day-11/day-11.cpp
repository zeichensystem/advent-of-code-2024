#include <numeric>
#include <optional>
#include <limits>
#include "aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/11
  
    Solutions: 
        - Part 1: 172484 (Example: 55312)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
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

int64_t part_one(const std::vector<std::string>& lines)
{   
    const std::vector<int64_t> stones = aocio::line_tokenise(lines.at(0), " ", "", [](const std::string& s) -> int64_t { return aocio::parse_num_i64(s).value(); });
    return apply_rules(stones, 25).size();
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