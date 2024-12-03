#include <numeric>
#include <array>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/2
  
    Solutions: 
        - Part 1: 432 (Example: 2)
        - Part 2: 488 (Example: 4)
    Notes:  
        - Part 1: 
        - Part 2: Was about to solve it "efficiently", but then just went with a simple quadratic brute-force solution 
                 (checking all possible report variations with one level removed until one of them, or none, is found to be safe).
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

bool report_is_safe_dampened(const std::vector<int>& levels)
{
    if (report_is_safe(levels)) { // Report is already safe without the "problem dampening".
        return true; 
    }

    // Try all variations of the report with one level removed until a safe report is found, or none is found (brute force with time O(n^2)).
    for (size_t remove_idx = 0; remove_idx < levels.size(); ++remove_idx) { 
        std::vector<int> one_removed;
        for (size_t idx = 0; idx < levels.size(); ++idx) { 
            if (idx != remove_idx) {
                one_removed.push_back(levels.at(idx));
            }
        }
        if (report_is_safe(one_removed)) { // Current report variation is safe.
            return true;
        }
    }
    
    return false; // No safe report variation was found.
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

int part_one(const std::vector<std::string>& lines, bool use_problem_dampener = false)
{
    std::vector<std::vector<int>> reports; 
    parse_reports(lines, reports);

    int safe_reports = 0; 
    for (const auto& report : reports) {
        if (!use_problem_dampener) {
            safe_reports += report_is_safe(report) ? 1 : 0;
        } else {
            safe_reports += report_is_safe_dampened(report) ? 1 : 0;
        }
    }
    return safe_reports;
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