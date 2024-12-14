#include <stack>
#include <numeric>
#include <unordered_set>
#include "aoclib/aocio.hpp"
#include "aoclib/grid.hpp"

/*
    Problem: https://adventofcode.com/2024/day/12
  
    Solutions: 
        - Part 1: 1415378 (Example: 1930)
        - Part 2:  862714 (Example: 1206)
    Notes:  
        - Part 1: 
        - Part 2:
*/

using aocutil::Grid; 
using Vec2 = aocutil::Vec2<int>;

struct Plot {
    Vec2 pos;
    int num_fences = 4; 
};

int region_price(const Grid<char>& grid, Grid<int>& visited, const Vec2& pos, bool discount = false)
{
    const char plant_sym = grid.at(pos);
    std::vector<Plot> plots; 

    std::stack<Vec2> s; 
    s.push(pos); 
    while (!s.empty()) { // DFS flood-fill.
        const Vec2 cur_pos = s.top(); 
        s.pop(); 
        if (visited.at(cur_pos) != 0) {
            continue;
        }
        visited.at(cur_pos) = 1;
        plots.push_back(Plot{.pos = cur_pos, .num_fences = 4});

        int num_adjacent = 0;
        constexpr std::array<Vec2, 4> dirs = aocutil::all_dirs_vec2<int>();
        for (const Vec2 dir: dirs) { // For all adjacent plots. 
            const Vec2 adj_pos = cur_pos + dir; 
            if (auto adj_sym = grid.try_get(adj_pos); adj_sym.has_value() && adj_sym.value() == plant_sym) {
                ++num_adjacent;
                if (!visited.at(adj_pos)) {
                    s.push(adj_pos);
                }
            }
        }
        assert(num_adjacent <= 4);
        plots.back().num_fences = 4 - num_adjacent;
    }

    const int area = std::ssize(plots);
    
    if (!discount) { // Part 1:
        int perimeter = std::reduce(plots.cbegin(), plots.cend(), int{0}, [](int sum, const Plot& p) {return sum + p.num_fences;});
        return area * perimeter; 
    }

    // Part 2:
    struct Edge {
        std::unordered_set<Vec2> points = {};
        Vec2 normal;
    };
    std::vector<Edge> edges; 
    for (const Vec2& normal : aocutil::all_dirs_vec2<int>()) {
        edges.push_back(Edge{.normal = normal});
    }

    const auto on_edge_with_normal = [&grid, plant_sym](const Vec2& pos, const Vec2& normal) {
        assert(grid.try_get(pos));
        return !grid.try_get(pos + normal) || grid.try_get(pos + normal).value() != plant_sym;
    };
    
    for (const Plot& plot : plots) {
        for (const Vec2& normal : aocutil::all_dirs_vec2<int>()) {
            if (!on_edge_with_normal(plot.pos, normal)) {
                continue;
            }
            [[maybe_unused]] bool found = false;
            for (Edge& e : edges) {
                if (e.normal == normal) {
                    e.points.insert(plot.pos);
                    found = true;
                    break;
                }
            }
            assert(found);
        }
    }

    int total_sides = 0;
    for (Edge& edge : edges) {
        const bool is_vertical_edge = edge.normal == Vec2{1, 0} || edge.normal == Vec2{-1, 0};

        std::unordered_set<int> xy_coords; 
        for (const Vec2& pt : edge.points) { // Get all unique x-coords for vertical edges (or all unique y-coords for horizontal edges).
            if (is_vertical_edge)
                xy_coords.insert(pt.x);
            else
                xy_coords.insert(pt.y);
        }

        for (int xy : xy_coords) { // For each unique x-coord for vertical edges (or for each unique y-coord for horizontal edges):
            std::vector<Vec2> line;
            std::copy_if(edge.points.cbegin(), edge.points.cend(), std::back_inserter(line), [is_vertical_edge, xy](const Vec2& a) {
                return is_vertical_edge ? xy == a.x : xy == a.y; // Get all points sharing the current unique x-coord/y-coord.
            });
            assert(line.size());
            std::sort(line.begin(), line.end(), [is_vertical_edge](const Vec2& a, const Vec2& b) {
                return is_vertical_edge ? a.y < b.y : a.x < b.x;
            });
            int current_sides = 1; 
            for (int i = 0; i < std::ssize(line) - 1; ++i) { 
                // If the sorted points are non-adjacent in the y-coord for vertical lines (or the x-coord for horizontal lines), we have multiple vertical/horizontal sides at the current x-coord/y-coord. 
                if ((is_vertical_edge && (std::abs(line.at(i).y - line.at(i + 1).y) > 1)) || (!is_vertical_edge && (std::abs(line.at(i).x - line.at(i + 1).x) > 1))) {
                    ++current_sides;
                }
            }
            total_sides += current_sides;
        }
    }
    return area * total_sides; 
}

int part_one(const std::vector<std::string>& lines, bool discount = false)
{
    const Grid<char> garden{lines}; 
    Grid<int> visited(garden.width(), garden.height(), 0); // Grid<int> and not Grid<bool> because std::vector<bool> is evil...
    
    int total_price = 0; 
    garden.foreach([&garden, &visited, &total_price, discount](const Vec2& pos, char elem) {
        if (!visited.at(pos)) {
            total_price += region_price(garden, visited, pos, discount); 
        }
    });
    return total_price;
}

int part_two(const std::vector<std::string>& lines)
{
    return part_one(lines, true);
}

int main(int argc, char* argv[])
{
    aocio::print_day();

    std::vector<std::string> pointss;
    const aocio::IOStatus status = aocio::handle_input(argc, argv, pointss);

    if (!aocio::iostat_has_flag(status, aocio::IOStatus::INPUT_SUCCESS)) {
        return EXIT_FAILURE;
    } else if (aocio::iostat_has_flag(status, aocio::IOStatus::INPUT_HELP)) {
        return EXIT_SUCCESS; 
    }

    try {
        auto p1 = part_one(pointss);  
        std::cout << "- Part 1: " << p1 << "\n";
        auto p2 = part_two(pointss);
        std::cout << "- Part 2: " << p2 << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << "Guru Meditation: " << err.what() << "\n";
        return EXIT_FAILURE;
    }
}