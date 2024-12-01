#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/N
  
    Solutions: 
        - Part 1: 
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

int part_one(const std::vector<std::string>& lines)
{
    return -1;
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