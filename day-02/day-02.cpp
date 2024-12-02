#include <numeric>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/2
  
    Solutions: 
        - Part 1: 432 (Example: 2)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

bool report_is_safe(const std::vector<int>& levels)
{
    if (levels.size() < 2) {
        throw std::invalid_argument("report_is_safe: report has fewer than 2 levels.");
        return false;
    }
    
    int num_valid = 0; 
    const bool must_be_decreasing = (levels.at(0) - levels.at(1)) > 0;

    for (size_t i = 0; i < levels.size() - 1; ++i) {
        int diff = levels.at(i) - levels.at(i + 1);
        bool is_decreasing = diff > 0;
        if ((!is_decreasing && must_be_decreasing) || (is_decreasing && !must_be_decreasing)) {
            return false;
        } else if (std::abs(diff) >= 1 && std::abs(diff) <= 3) {
            ++num_valid;
        } else {
            return false;
        }
    } 
    assert(num_valid == std::ssize(levels) - 1);
    return true;
}

void parse_reports(const std::vector<std::string>& lines, std::vector<std::vector<int>>& reports)
{
    for (const std::string& line : lines) {
        std::vector<std::string> toks; 
        aocio::line_tokenise(line, " ", "", toks);
        if (toks.size() < 2) {
            throw std::invalid_argument("parse_lines: Level has fewer than 2 reports.");
        } 
        reports.push_back(std::vector<int>{});
        for (const std::string& tok : toks) {
            int level = aocio::parse_num(tok).value();
            reports.back().push_back(level);
        }
    }
}

int part_one(const std::vector<std::string>& lines)
{
    std::vector<std::vector<int>> reports; 
    parse_reports(lines, reports);

    int safe_reports = 0; 
    for (const auto& report : reports) {
        // std::cout << (report_is_safe(report) ? "Safe" : "Unsafe") << "\n";
        safe_reports += report_is_safe(report) ? 1 : 0;
    }
    return safe_reports;
}

int part_two(const std::vector<std::string>& lines)
{
    return -1; 
}

int main(int argc, char* argv[])
{
    aocio::print_day();

    std::vector<std::string> lines;
    if (!aocio::handle_input(argc, argv, lines)) {
        return EXIT_FAILURE;
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