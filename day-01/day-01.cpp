#include <algorithm>
#include <numeric>
#include <unordered_map>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/1
  
    Solutions: 
        - Part 1: 2113135
        - Part 2: 19097157
    Notes:  
        - Part 1:
        - Part 2: 
*/

void parse_lists(const std::vector<std::string>& lines, std::vector<int>& id_1, std::vector<int>& id_2)
{
    for (const auto& line : lines) {
        std::vector<std::string> toks;
        aocio::line_tokenise(line, " ", "", toks);
        id_1.push_back(aocio::parse_num(toks.at(0)).value()); 
        id_2.push_back(aocio::parse_num(toks.at(1)).value());
    }
    if (id_1.size() != id_2.size()) {
        throw std::invalid_argument("parse_lists: id-lists of unequal length!");
    }
}

int part_one(const std::vector<std::string>& lines)
{
    std::vector<int> id_1, id_2; 
    parse_lists(lines, id_1, id_2); 

    std::sort(id_1.begin(), id_1.end());
    std::sort(id_2.begin(), id_2.end());

    std::vector<int> diffs;
    std::transform(id_1.cbegin(), id_1.cend(), id_2.cbegin(), std::back_inserter(diffs), [](int a, int b) -> int {
        return std::abs(a - b);
    });
    
    return std::accumulate(diffs.cbegin(), diffs.cend(), 0);    
}

int part_two(const std::vector<std::string>& lines)
{
    std::vector<int> id_1, id_2; 
    parse_lists(lines, id_1, id_2); 

    std::unordered_map<int, int> count; 
    for (int id : id_2) {
        if (count.contains(id)) {
            count.at(id) += 1; 
        } else {
            count.insert({id, 1});
        }
    }

    int score = 0; 
    for (int id : id_1) {
        score += id * (count.contains(id) ? count.at(id) : 0); 
    }
    return score;
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