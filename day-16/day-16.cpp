#include <queue>
#include <unordered_set>
#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/16
  
    Solutions: 
        - Part 1: 123540 (Example: 7036)
        - Part 2:    665 (Example:   45)
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;
using aocutil::Grid;

typedef int score_int_t;
constexpr score_int_t INFINITY_SCORE = std::numeric_limits<score_int_t>::max();

struct ReindeerState 
{
    Vec2 pos, dir;
    score_int_t score = 0;

    bool operator==(const ReindeerState& other) const 
    {
        return pos == other.pos && dir == other.dir;
    }
}; 

class StateCell 
{
    std::vector<ReindeerState> states;
    std::vector<std::vector<ReindeerState>> prev_states;

    public:
    score_int_t get_cost(const ReindeerState& state) const
    {
        const auto it = std::find(states.cbegin(), states.cend(), state); 
        if (it == states.cend()) {
            return INFINITY_SCORE;
        } else {
            return it->score;
        }
    }

    void update_cost(const ReindeerState& state)
    {
        const auto it = std::find(states.begin(), states.end(), state); 
        if (it == states.end()) {
            states.push_back(state);
            prev_states.push_back({});
        } else {
            it->score = state.score;
        }
        assert(states.size() <= 4);
        assert(states.size() == prev_states.size());
    }

    bool contains(const ReindeerState& state) const
    {
        return std::find(states.cbegin(), states.cend(), state) != states.cend(); 
    }

    std::vector<ReindeerState>& get_prev_states(const ReindeerState& state) 
    {
        const auto it = std::find(states.cbegin(), states.cend(), state); 
        if (it == states.cend()) {
            throw std::runtime_error("get_prev_states: state not in .states");
        } else {
            std::ptrdiff_t idx = it - states.cbegin();
            return prev_states.at(idx);
        }
    }
};

score_int_t find_cheapest_path(const Grid<char>& map, const Vec2& start_pos, const Vec2& end_pos, std::unordered_set<Vec2>* shortest_paths_tiles = nullptr)
{
    const auto adjacent_states = [&map](const ReindeerState& r) {
        constexpr score_int_t TURN_COST = 1000, FORWARD_COST = 1;
        std::vector<ReindeerState> adjacent;

        const Vec2 dir_left = r.dir.perp_dot(), dir_right = r.dir.perp_dot(false);
        std::array<ReindeerState, 3> adj_candidates = {
            ReindeerState {.pos = r.pos,         .dir = dir_left,  .score = r.score + TURN_COST   }, // Turn Left.
            ReindeerState {.pos = r.pos,         .dir = dir_right, .score = r.score + TURN_COST   }, // Turn Right.
            ReindeerState {.pos = r.pos + r.dir, .dir = r.dir,     .score = r.score + FORWARD_COST}  // Move Forwards.
        };

        for (const auto& adj : adj_candidates) {
            if (const auto tile = map.try_get(adj.pos); tile && tile.value() != '#') {
                adjacent.push_back(adj);
            }
        }
        return adjacent;
    };

   /*
        Note: Dijkstra implemented using std::priority_queue which does not have "update-priority" functionality:
        cf. https://github.com/zeichensystem/advent-of-code-2023/blob/main/day-17/day-17.cpp#L142 (last retrieved 2024-12-18)
            https://cs.stackexchange.com/questions/118388/dijkstra-without-decrease-key (last retrieved 2024-12-18)
    */
    const auto prio_cmp = [](const ReindeerState& a, const ReindeerState& b) { return a.score > b.score; }; // Min-prio-queue (.top() yields state with lowest score)
    std::priority_queue<ReindeerState, std::vector<ReindeerState>, decltype(prio_cmp)> queue(prio_cmp);
    queue.push(ReindeerState{.pos = start_pos, .dir = {1, 0}, .score = 0});

    Grid<StateCell> state_grid(map.width(), map.height(), StateCell{});
    state_grid.at(start_pos).update_cost(queue.top());

    while (!queue.empty()) { 
        const ReindeerState current_state = queue.top();
        queue.pop();

        const int current_score_queue = current_state.score;
        const int current_score = state_grid.at(current_state.pos).get_cost(current_state); // The actual score.

        if (current_score_queue != current_score) { // The current state was already in queue with a lower score (i.e. higher priority); cf. links above.
            assert(current_score_queue > current_score);
            continue;
        }

        if (current_state.pos == end_pos) {
            if (shortest_paths_tiles) { // Part 2: Depth-first traversal of the previous states.
                std::stack<ReindeerState> s;
                s.push(current_state);
                while (!s.empty()) {  
                    const ReindeerState state = s.top(); 
                    s.pop();
                    shortest_paths_tiles->insert(state.pos);
                    for (ReindeerState prev_state : state_grid.at(state.pos).get_prev_states(state)) {
                        s.push(prev_state);
                    }
                }
            }
            return current_score;
        }

        for (ReindeerState adj_state : adjacent_states(current_state)) {
            const score_int_t current_min_score = state_grid.at(adj_state.pos).get_cost(adj_state);
            if (adj_state.score < current_min_score) {
                state_grid.at(adj_state.pos).update_cost(adj_state);
                queue.push(adj_state); // std::priority_queue has no "update priority" functionality, thus we just push a new state with the new, lower score; cf. links above.
            } 
            if (adj_state.score <= current_min_score && shortest_paths_tiles)  { // Part 2: ('<=' and not '<' because we need the prev states for all shortest paths and not just one).
                std::vector<ReindeerState>& prev_states = state_grid.at(adj_state.pos).get_prev_states(adj_state);
                if (adj_state.score < current_min_score) {
                    prev_states.clear();
                }
                prev_states.push_back(current_state);
            }
        }
    }
    std::cerr << "find_cheapest_path: Did not find path to end_pos.\n";
    return -1;
}

auto part_one(const std::vector<std::string>& lines)
{
    const Grid<char> map(lines);     
    const Vec2 start_pos = map.find_elem_positions('S').at(0), end_pos = map.find_elem_positions('E').at(0);
    return find_cheapest_path(map, start_pos, end_pos);
}

auto part_two(const std::vector<std::string>& lines)
{
    const Grid<char> map(lines);     
    const Vec2 start_pos = map.find_elem_positions('S').at(0), end_pos = map.find_elem_positions('E').at(0);
    std::unordered_set<Vec2> shortest_paths_tiles;
    find_cheapest_path(map, start_pos, end_pos, &shortest_paths_tiles);
    return shortest_paths_tiles.size();
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