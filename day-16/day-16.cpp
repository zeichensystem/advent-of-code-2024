#include <queue>
#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/16
  
    Solutions: 
        - Part 1: 123540 (Example: 7036)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;
using aocutil::Grid;
typedef int score_int_t;

struct ReindeerState {
    Vec2 pos, dir;
    score_int_t score = 0;
}; 

constexpr score_int_t INFINITY = std::numeric_limits<score_int_t>::max();

score_int_t find_cheapest_path(const Grid<char>& map, const Vec2& start_pos, const Vec2& end_pos)
{
    const auto adjacent_states = [&map](const ReindeerState& r) {
        constexpr score_int_t TURN_COST = 1000, FORWARD_COST = 1;
        std::vector<ReindeerState> adjacent;

        const Vec2 dir_left = r.dir.perp_dot(), dir_right = r.dir.perp_dot(false);
        std::array<ReindeerState, 3> adj_candidates = {
            ReindeerState {.pos = r.pos + dir_left,  .dir = dir_left,  .score = r.score + TURN_COST + FORWARD_COST}, // Left
            ReindeerState {.pos = r.pos + dir_right, .dir = dir_right, .score = r.score + TURN_COST + FORWARD_COST}, // Right
            ReindeerState {.pos = r.pos + r.dir,     .dir = r.dir,     .score = r.score + FORWARD_COST},  // Forwards
        };

        for (const auto& adj : adj_candidates) {
            if (const auto tile = map.try_get(adj.pos); tile && tile.value() != '#') {
                adjacent.push_back(adj);
            }
        }
        return adjacent;
    };

    Grid<score_int_t> cost_grid(map.width(), map.height(), INFINITY);
    cost_grid.at(start_pos) = 0;

   /*
        Note:
        Dijkstra implemented using a priority queue without "decrease-priority" functionality:
        cf. https://github.com/zeichensystem/advent-of-code-2023/blob/main/day-17/day-17.cpp#L142 
            https://cs.stackexchange.com/questions/118388/dijkstra-without-decrease-key (last retrieved 2024-12-18)
    */
    const auto prio_cmp = [](const ReindeerState& a, const ReindeerState& b) { return a.score > b.score; }; // Min-prio-queue (.top() yields state with lowest score)
    std::priority_queue<ReindeerState, std::vector<ReindeerState>, decltype(prio_cmp)> queue(prio_cmp);
    queue.push(ReindeerState{.pos = start_pos, .dir = {1, 0}, .score = 0});

    while (!queue.empty()) { 
        const ReindeerState current_state = queue.top();
        queue.pop();

        const int current_score_queue = current_state.score;
        const int current_score = cost_grid.at(current_state.pos); // The actual score.

        if (current_score_queue != current_score) { // The current state was already in queue with a lower score (i.e. higher priority); cf. links above.
            assert(current_score_queue > current_score);
            continue;
        }

        if (current_state.pos == end_pos) {
            return current_state.score;
        }

        for (ReindeerState adj_state : adjacent_states(current_state)) {
            if (adj_state.score < cost_grid.at(adj_state.pos)) {
                cost_grid.at(adj_state.pos) = adj_state.score;
                queue.push(adj_state); // std::priority_queue has no "update priority" functionality, thus we just push a new state with the new, lower score; cf. links above.
            }
        }
    }
    std::cerr << "Found no path to end_pos.\n";
    return -1;
}

int part_one(const std::vector<std::string>& lines)
{
    const Grid<char> map(lines);     
    const Vec2 start_pos = map.find_elem_positions('S').at(0);
    const Vec2 end_pos = map.find_elem_positions('E').at(0);
    return find_cheapest_path(map, start_pos, end_pos);
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