#include <numeric>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/3
  
    Solutions: 
        - Part 1: 182780583 (Example: 161)
        - Part 2:  90772405 (Example:  48)
    Notes:  
        - Part 1: 
        - Part 2:
*/

int part_one(const std::vector<std::string>& lines, bool part_two = false)
{
    constexpr int MAX_DIGITS = 3;
    constexpr int NUM_OPERANDS = 2;
    const std::string input = aocio::str_without_whitespace(
        std::reduce(lines.cbegin(), lines.cend(), std::string{""}, [](const std::string& a, const std::string& b) -> std::string { return a + b; })
    );
    
    int mul_sum = 0; 
    std::string::size_type i_start = 0;

    while (i_start < input.size()) {
        i_start = input.find("mul(", i_start);
        if (i_start == std::string::npos) {
            return mul_sum;
        }

        bool mul_enabled = true;
        if (part_two) {
            const auto dont_i = input.rfind("don't()", i_start);
            const auto do_i = input.rfind("do()", i_start);
            if (dont_i != std::string::npos) {
                if (do_i != std::string::npos && do_i > dont_i) {
                    mul_enabled = true;
                } else {
                    mul_enabled = false;
                }
            }
        }

        const auto parse_operand = [&input, &i_start](bool is_last) -> std::optional<int> { // Gets the next operand inside mul(
            int num = 0; 
            int n_digits = 0; 
            while (i_start < input.size()) {
                const auto digit = aocio::parse_digit(input.at(i_start)); 
                if (!digit.has_value()) {
                    if ((!is_last && input.at(i_start) == ',') || (is_last && input.at(i_start) == ')')) {
                        ++i_start;
                        return num;
                    } 
                    return {};
                }
                num = num * 10 + digit.value(); 
                ++n_digits; 
                ++i_start;
                if (n_digits > MAX_DIGITS) {
                    return {};
                }
            }
            return {};
        };

        // std::cout << input.substr(i_start, 12) << "\n";
        i_start += 4; // Advance to the first char after 'mul('

        for (int operand_i = 0, mul_res = 1; operand_i < NUM_OPERANDS; ++operand_i) {
            bool is_last = operand_i == (NUM_OPERANDS - 1);
            auto op = parse_operand(is_last);
            if (!op.has_value()) {
                break;
            }
            // std::cout << "op: " << op.value() << "\n";
            mul_res *= op.value();
            if (is_last && mul_enabled) {
                mul_sum += mul_res;
            }
        }
    }

    return mul_sum;
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