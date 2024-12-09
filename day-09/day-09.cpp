#include <vector>
#include <numeric>
#include <set>
#include "../aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/9
  
    Solutions: 
        - Part 1: 6262891638328 (Example: 1928)
        - Part 2: 6287317016845 (Example: 2858)
    Notes:  
        - Part 1: 
        - Part 2: Phew, this is not pretty. 
*/


struct IDRange {
    using Int = int64_t;
    static constexpr Int ID_FREE = -1;
    Int id;
    Int start, size;

    friend bool operator<(const IDRange& lhs, const IDRange& rhs) {
        return lhs.start < rhs.start;
    };

    friend std::ostream& operator<<(std::ostream& os, const IDRange &a) {
        return os << "id " << a.id << " start " << a.start << " size " << a.size << "\n";
    }   
};

using Int = IDRange::Int;

auto part_one(const std::vector<std::string>& lines)
{
    std::vector<Int> disk; 
    std::transform(lines.at(0).cbegin(), lines.at(0).cend(), std::back_inserter(disk), [](char c) -> Int { return aocio::parse_digit(c).value(); });
   
    std::set<IDRange> id_ranges;
    Int range_start = 0;
    for (Int i = 0; i < std::ssize(disk); ++i) {
        Int id = i / 2; 
        id = (i % 2) ? IDRange::ID_FREE : id;
        IDRange range = {.id = id, .start = range_start, .size = disk.at(i)};
        if (range.size) {
            id_ranges.insert(range);
        }
        range_start += range.size;
    }

    const auto find_first_free = [&id_ranges]() {
        return std::find_if(id_ranges.cbegin(), id_ranges.cend(), [](auto& range) -> bool {return range.id == IDRange::ID_FREE;});
    };
    const auto find_last_occupied = [&id_ranges]() {
        return std::find_if(id_ranges.crbegin(), id_ranges.crend(), [](auto& range) -> bool {return range.id != IDRange::ID_FREE;});
    };
    auto free_it = find_first_free(); 
    auto occupied_it = find_last_occupied(); 
    
    for (; free_it != id_ranges.cend() && occupied_it != id_ranges.crend(); free_it = find_first_free(), occupied_it = find_last_occupied()) {
        const IDRange& free = *free_it;
        const IDRange& occupied = *occupied_it;

        if (free.start + free.size > occupied.start) {
            break;
        }

        IDRange new_occupied = {.size = 0}, new_free = {.size = 0};
        if (free.size < occupied.size) { // 1.) Last range will be non-empty (and the whole free range will be filled)
            new_occupied = occupied; 
            new_free = free;
            new_occupied.size -= free.size;
            new_free.id = occupied.id; 
        } else if (occupied.size < free.size) { // 2.) Last range will be empty (and the free range will have some space left)
            new_occupied = {.id = occupied.id, .start = free.start, .size = occupied.size};
            new_free = {.id = IDRange::ID_FREE, .start = new_occupied.start + new_occupied.size, .size = free.size - occupied.size};
        } else { // 3.) Last range fits exactly into free range.
            new_occupied = free;
            new_occupied.id = occupied.id;
        }
        id_ranges.erase(free);
        id_ranges.erase(occupied);
        if (new_free.size) {
            id_ranges.insert(new_free);
        }
        if (new_occupied.size) {
            id_ranges.insert(new_occupied);
        }
    }

    const auto sum_of_first_n = [](Int n) -> Int {
        return n <= 0 ? 0 : n * (n + 1) / 2;
    };

    Int result = 0;
    for (const IDRange& range : id_ranges) {
        if (range.id == IDRange::ID_FREE) {
            continue;
        }
        Int index_sum = sum_of_first_n(range.start + range.size - 1) - sum_of_first_n(range.start - 1);
        result += index_sum * range.id;
        
    }
    return result;
}

auto part_two(const std::vector<std::string>& lines)
{
    std::vector<Int> disk; 
    std::transform(lines.at(0).cbegin(), lines.at(0).cend(), std::back_inserter(disk), [](char c) -> Int { return aocio::parse_digit(c).value(); });
   
    std::set<IDRange> id_ranges;
    Int range_start = 0;
    for (Int i = 0; i < std::ssize(disk); ++i) {
        Int id = i / 2; 
        id = (i % 2) ? IDRange::ID_FREE : id;
        IDRange range = {.id = id, .start = range_start, .size = disk.at(i)};
        if (range.size) {
            id_ranges.insert(range);
        }
        range_start += range.size;
    }

    const auto find_first_free = [&id_ranges](const auto cbegin) {
        return std::find_if(cbegin, id_ranges.cend(), [](auto& range) -> bool {return range.id == IDRange::ID_FREE;});
    };

    std::vector<IDRange> files;
    std::copy_if(id_ranges.crbegin(), id_ranges.crend(), std::back_inserter(files), [](const auto& range) -> bool {return range.id != IDRange::ID_FREE;});

    for (const IDRange occupied : files) {

        auto free_it = find_first_free(id_ranges.cbegin()); 

        if (free_it == id_ranges.cend() || free_it->start + free_it->size > occupied.start) {
            break;
        }
        
        while (free_it != id_ranges.cend() && free_it->size < occupied.size) {
            free_it = find_first_free(std::next(free_it, 1)); 
        }

        if (free_it == id_ranges.cend() || free_it->start >= occupied.start) { // None was fitting -> leave the block.
            continue;
        } 

        const IDRange& free = *free_it;
        IDRange new_occupied = {.size = 0}, new_free = {.size = 0};
        assert(occupied.size <= free.size);

       if (occupied.size < free.size) { // Last range will be empty (and the free range will have some space left)
            new_occupied = {.id = occupied.id, .start = free.start, .size = occupied.size};
            new_free = {.id = IDRange::ID_FREE, .start = new_occupied.start + new_occupied.size, .size = free.size - occupied.size};
        } else { // Last range fits exactly into free range.
            new_occupied = free;
            new_occupied.id = occupied.id;
            // new_free = occupied;
            // new_free.id = IDRange::ID_FREE;
        }

        id_ranges.erase(free);
        id_ranges.erase(occupied);

        if (new_free.size) {
            id_ranges.insert(new_free);
        }
        if (new_occupied.size) {
            id_ranges.insert(new_occupied);
        }
    }

    const auto sum_of_first_n = [](Int n) -> Int {
        return n <= 0 ? 0 : n * (n + 1) / 2;
    };

    Int result = 0;
    for (const IDRange& range : id_ranges) {
        if (range.id == IDRange::ID_FREE) {
            continue;
        }
        Int index_sum = sum_of_first_n(range.start + range.size - 1) - sum_of_first_n(range.start - 1);
        result += index_sum * range.id;
    }
    return result;
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