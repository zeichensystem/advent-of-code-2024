#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/5
  
    Solutions: 
        - Part 1: 4609 (Example: 143)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

void parse_input(const std::vector<std::string>& lines, std::unordered_map<int, std::unordered_set<int>>& ordering_rules, std::vector<std::vector<int>>& updates)
{
    bool first_section = true;
    for (const auto& line: lines) {
        if (aocio::str_without_whitespace(line) == "") { 
            first_section = false;
            continue;
        }
        if (first_section) { // 1.) Parse the page-ordering-rules.
            std::vector<std::string> toks;
            aocio::line_tokenise(line, "|", "", toks);
            if (toks.size() != 2) {
                throw std::invalid_argument("parse_input: Invalid page ordering rule.");
            }
             // after -> [before_1, before_2, ...]
            int before = aocio::parse_num(toks.at(0)).value();
            int after = aocio::parse_num(toks.at(1)).value();
            if (ordering_rules.contains(after)) {
                ordering_rules.at(after).insert(before); 
            } else {
                ordering_rules.insert({after, std::unordered_set<int>{before}});
            }
        } else { // 2.) Parse the page numbers of each update.
            std::vector<std::string> toks;
            aocio::line_tokenise(line, ",", "", toks);
            if (toks.size() < 1) {
                throw std::invalid_argument("parse_input: Invalid update.");
            }
            updates.push_back(std::vector<int>{});
            for (const auto& tok : toks) {
                updates.back().push_back(aocio::parse_num(tok).value());
            }
        }
    }
    if (!updates.size()) {
        throw std::invalid_argument("parse_input: Input contains no updates.");
    }
}

int part_one(const std::vector<std::string>& lines)
{
    std::unordered_map<int, std::unordered_set<int>> ordering_rules; // page_n -> [before_1, before_2, ...]
    std::vector<std::vector<int>> updates; 
    parse_input(lines, ordering_rules, updates);
    int sum_of_middle_pages = 0;

    for (const auto& pages : updates) {
        for (auto it = pages.cbegin(); it != pages.cend(); ++it) {
            const int page = *it;
            if (ordering_rules.contains(page)) { // The current page has "prerequisites" (pages which must come before it).
                const std::unordered_set<int>& before = ordering_rules.at(page); 
                for (auto after_it = it; after_it != pages.cend(); ++after_it) {
                    if (before.contains(*after_it)) { // A page which must come before the current page actually comes after it -> violation of the ordering rules.
                        goto next;
                    }
                }
            }
        }
        if (pages.size() % 2 == 0) { // Only can find a middle value if the number of pages is uneven. 
            throw std::invalid_argument("part_one: page has valid order, but does not have a middle value");
        }
        sum_of_middle_pages += pages.at(pages.size() / 2);
        next:;
    }

    return sum_of_middle_pages;
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