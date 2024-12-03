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
    constexpr int MUL_MAX_DIGITS = 3, MUL_NUM_OPERANDS = 2;
    const std::string input = aocio::str_without_whitespace(std::reduce(lines.cbegin(), lines.cend(), std::string{""}, std::plus<std::string>{}));
    int total_mul_sum = 0; 

    for (auto idx = input.find("mul("); idx < input.size(); idx = input.find("mul(", idx)) {
        bool mul_enabled = true;
        if (part_two) {
            const auto dont_idx = input.rfind("don't()", idx), do_idx = input.rfind("do()", idx);
            if (dont_idx != std::string::npos) {
                if (do_idx != std::string::npos && do_idx > dont_idx) { // The most recent do/don't instruction before the current "mul(" was "do"
                    mul_enabled = true;
                } else { // The most recent do/don't instruction before the current "mul(" was "don't"
                    mul_enabled = false;
                }
            }
        }

        idx += 4; // Make idx point to the first digit of the current mul-instruction's first operand (assuming the current mul-instruction will be valid).

        const auto parse_operand = [&input, &idx](bool is_last_op) -> std::optional<int> {  
            // Parses an operand inside a mul-instruction (assumes idx points to the first character of the operand to be parsed).
            // Advances idx to point to the first character of the next operand (or to the character after the closing parenthesis if it was the last operand, or to the offending invalid char).
            int operand = 0; 
            for (int n_digits = 0; idx < input.size() && n_digits <= MUL_MAX_DIGITS; ++n_digits) {
                const char c = input.at(idx); 
                if (const auto digit = aocio::parse_digit(c); digit.has_value()) {
                    operand = operand * 10 + digit.value();
                    ++idx;
                } else {
                    const bool is_valid_end_char = (!is_last_op && c == ',') || (is_last_op && c == ')');
                    return (n_digits && is_valid_end_char) ? (++idx, operand) : std::optional<int>{}; // Only increment idx if the operand was valid.
                } 
            }
            return {};
        };

        for (int op_i = 0, mul = 1; op_i < MUL_NUM_OPERANDS; ++op_i) { // Try to parse the operands of the current mul instruction.
            const bool is_last_op = op_i == (MUL_NUM_OPERANDS - 1);
            if (const auto op = parse_operand(is_last_op); op.has_value()) {
                mul *= op.value();
                total_mul_sum = (is_last_op && mul_enabled) ? total_mul_sum + mul : total_mul_sum;
            } else {
                break;
            }
        }
    }
    return total_mul_sum;
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