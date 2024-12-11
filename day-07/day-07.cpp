#include <numeric>
#include "aoclib/aocio.hpp"
#include "aoclib/parallel.hpp"

/*
    Problem: https://adventofcode.com/2024/day/7
  
    Solutions: 
        - Part 1:  6083020304036  (Example:  3749)
        - Part 2: 59002246504791  (Example: 11387)
    Notes:  
        - Part 1: - Recursive backtracking for equation_has_solution()
        - Part 2: - Only needed to define concat_op()
                  - Implemented parallel_transform_reduce as an exercise (improved perf from ~130 ms to ~68 ms on my laptop)
*/

struct Equation {
    int64_t result; 
    std::vector<int64_t> operands;
};

std::vector<Equation> parse_equations(const std::vector<std::string>& lines)
{
    std::vector<Equation> equations;
    for (const auto& line : lines) {
        std::vector<std::string> toks;
        aocio::line_tokenise(line, ": ", ":", toks);
        if (toks.size() < 3) {
            throw std::invalid_argument("parse_equation: Invalid equation.");
        } else if (toks.at(1) != ":") {
            throw std::invalid_argument("parse_equation: Invalid equation.");
        }

        Equation eq; 
        if (auto result = aocio::parse_num_i64(toks.front()); result.has_value()) {
            eq.result = result.value();
        } else {
            throw std::invalid_argument("parse_equation: Equation result not a number.");
        }
        for (size_t i = 2; i < toks.size(); ++i) {
            if (auto operand = aocio::parse_num_i64(toks.at(i)); operand.has_value()) {
                eq.operands.push_back(operand.value());
            } else {
                throw std::invalid_argument("parse_equation: Equation pperand not a number.");
            }
        }
        if (eq.operands.size() < 2) {
            throw std::invalid_argument("parse_equation: Not enough operands.");
        }
        equations.push_back(eq);
    }
    return equations;
}

using BinaryOperator = const std::function<int64_t(int64_t, int64_t)>;

bool equation_has_solution(const std::vector<BinaryOperator>& operators, const Equation& eq, size_t operand_idx = 0, int64_t result = 0) 
{
    if (operand_idx == eq.operands.size()) {
        return result == eq.result;  
    } else if (operand_idx == 0) {
        return equation_has_solution(operators, eq, 1, eq.operands.at(0));
    }

    for (const auto& op : operators) {
        auto current_result = op(result, eq.operands.at(operand_idx));
        if (equation_has_solution(operators, eq, operand_idx + 1, current_result)) {
            return true;
        }
    }

    return false;
}

int64_t concat_op(int64_t lhs, int64_t rhs)
{
    assert(lhs > 0 && rhs >= 0);
    int64_t n = 10;
    while (n <= rhs) {
        n *= 10;
    }
    return lhs * n + rhs;
}

int64_t part_one(const std::vector<std::string>& lines, bool part_two = false)
{
    const std::vector<Equation> equations = parse_equations(lines);
    std::vector<BinaryOperator> operators = !part_two ? std::vector<BinaryOperator>{std::plus{}, std::multiplies{}} 
                                                      : std::vector<BinaryOperator>{std::plus{}, std::multiplies{}, concat_op};

    return aocutil::parallel_transform_reduce(equations.cbegin(), equations.cend(), int64_t{0}, std::plus{}, [&operators](const Equation& eq) -> int64_t {
        return equation_has_solution(operators, eq) ? eq.result : 0;
    });
}

int64_t part_two(const std::vector<std::string>& lines)
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
        int64_t p1 = part_one(lines);  
        std::cout << "- Part 1: " << p1 << "\n";
        int64_t p2 = part_two(lines);
        std::cout << "- Part 2: " << p2 << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << "Guru Meditation: " << err.what() << "\n";
        return EXIT_FAILURE;
    }
}