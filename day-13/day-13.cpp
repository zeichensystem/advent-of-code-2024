#include "matrix.hpp"
#include "aoclib/aocio.hpp"
#include "aoclib/vec.hpp"

/*
    Problem: https://adventofcode.com/2024/day/13
  
    Solutions: 
        - Part 1:           26299 (Example:          480)
        - Part 2: 107824497933339 (Example: 875318608908)
    Notes:  
        - Part 1: - Wrote a recursive-descent parser as an exercise.
                  - Implemented a reduced_row_echelon function in matrix.hpp to solve linear equation systems
        - Part 2: - I'm glad I did not run into floating point accuracy issues with double...
*/

using Vec2 = aocutil::Vec2<int64_t>;

struct ClawMachine 
{
    static constexpr int cost_btn_a = 3, cost_btn_b = 1;
    Vec2 delta_btn_a, delta_btn_b;
    Vec2 prize_location;

    friend std::ostream& operator<<(std::ostream& os, const ClawMachine& cm)
    {
        os << "Button A: " << cm.delta_btn_a << "\n";
        os << "Button B: " << cm.delta_btn_b << "\n";
        return os << "Prize: " << cm.prize_location << "\n";
    }
};

class ClawMachineParser : public aocio::RDParser 
{
    public:
    ClawMachineParser(const std::vector<std::string>& lines) : aocio::RDParser(lines, "+-:=, \t", "+-:=,") {}

    std::vector<ClawMachine> parse()
    {
        std::vector<ClawMachine> result;
        while (!is_end()) {            
            result.push_back(require_claw_machine());
            bool sep = false; 
            while (accept_newline()) {
                sep = true;
            };
            if (!sep && !is_end()) {
                throw std::runtime_error("Parser::parse: Missing empty line between claw machine definitions.");
            }
        }
        return result;
    }

    private:
    Vec2 require_coords()
    {
        std::optional<Vec2::value_type> x, y;
        for (int i = 0; i < 2; ++i) {
            std::string_view x_y = current_token();
            if (x_y == "X" && x.has_value()) {
                throw std::runtime_error("Parser::button_coord: x-coordinate already defined.");
            } else if (x_y == "Y" && y.has_value()) {
                throw std::runtime_error("Parser::button_coord: y-coordinate already defined.");
            }
            require_one_of_tokens({"X", "Y"});
            accept_token("=");
            Vec2::value_type sign = 1;
            if (accept_token("+")) {
                sign = 1;
            } else if (accept_token("-")) {
                sign = -1;
            }
            Vec2::value_type num = require_int<Vec2::value_type>();
            if (x_y == "X") {
                x = num * sign;
            } else {
                y = num * sign;
            }
            if (i == 0) {
                require_token(",");
            } 
        }
        return Vec2{x.value(), y.value()};
    }

    std::optional<std::string_view> accept_button(ClawMachine &cm, std::string_view a_b_existing = "")
    {
        if (!accept_token("Button")) {
            return {};
        }
        std::string_view a_b = current_token();
        if (a_b_existing == "A") {
            require_token("B");
        } else if (a_b_existing == "B") {
            require_token("A");
        } else {
            require_one_of_tokens({"A", "B"});
        }
        Vec2& btn = a_b == "A" ? cm.delta_btn_a : cm.delta_btn_b;
        require_token(":");
        btn = require_coords();
        return a_b;
    }

    void require_prize(ClawMachine& cm)
    {
        require_token("Prize");
        require_token(":"); 
        cm.prize_location = require_coords();
    }

    ClawMachine require_claw_machine()
    {
        ClawMachine cm;
        std::string_view btn_name_read = "";
        bool price_read = false;
        int btns_read = 0;
        for (int i = 0; i < 3; ++i) {
            if (const auto btn_name = accept_button(cm, btn_name_read); btn_name.has_value()) { // Button.
                if (btn_name.value() == btn_name_read) {
                    throw std::runtime_error("Parser::claw_machine: Button already defined.");
                }
                ++btns_read;
            } else { // Not a button, must be a price.
                require_prize(cm);
                if (price_read) {
                    throw std::runtime_error("Parser::claw_machine: Price already defined.");
                }
                price_read = true;
            }
            accept_newline();
        }
        if (btns_read < 2) {
            throw std::runtime_error("Parser::claw_machine: Missing buttons.");
        } else if (btns_read > 2) {
            throw std::runtime_error("Parser::claw_machine: Too many buttons.");
        }
        if (!price_read) {
            throw std::runtime_error("Parser::claw_machine: No price.");
        }
        return cm;
    }
};

Vec2::value_type part_one(const std::vector<std::string>& lines, bool part_two = false)
{
    ClawMachineParser parser(lines);
    std::vector<ClawMachine> machines = parser.parse();

    // Approach: Solve the following linear system of equations (if possible) and only consider integer solutions
    // (1) delta_btn_a.x * a + delta_btn_b.x * b = prize_location.x
    // (2) delta_btn_a.y * a + delta_btn_b.y * b = prize_location.y

    return std::transform_reduce(machines.cbegin(), machines.cend(), Vec2::value_type{0}, std::plus{}, [part_two](const ClawMachine& cm) -> Vec2::value_type {
        const Vec2 prize_location = !part_two ? cm.prize_location : cm.prize_location + Vec2{10000000000000, 10000000000000};
        Matrix<double, 2, 3> mat(cm.delta_btn_a, cm.delta_btn_b, prize_location);
        if (mat.reduced_row_echelon()) { // Has a solution.
            Vec2::value_type a = std::round(mat.at(0, 2));
            Vec2::value_type b = std::round(mat.at(1, 2));
            if (std::round(mat.at(1, 1)) == 0) { // Infinite solutions (we are only interested in the lowest, i.e. we pick b = 0)
                assert(b == 0);
            } 
            if (a * cm.delta_btn_a + b * cm.delta_btn_b == prize_location) { // Has an integer solution.
                return cm.cost_btn_a * a + cm.cost_btn_b * b;
            } 
        } 
        return 0; // No solution (or no integer solution).
    });
}

auto part_two(const std::vector<std::string>& lines)
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