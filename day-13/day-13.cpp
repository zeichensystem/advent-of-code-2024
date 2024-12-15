#include <unordered_map>
#include "aoclib/aocio.hpp"
#include "aoclib/vec.hpp"


/*
    Problem: https://adventofcode.com/2024/day/13
  
    Solutions: 
        - Part 1: 
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

using Vec2 = aocutil::Vec2<int>;

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
        std::optional<int> x, y;
        for (int i = 0; i < 2; ++i) {
            std::string_view x_y = current_token();
            if (x_y == "X" && x.has_value()) {
                throw std::runtime_error("Parser::button_coord: x-coordinate already defined.");
            } else if (x_y == "Y" && y.has_value()) {
                throw std::runtime_error("Parser::button_coord: y-coordinate already defined.");
            }
            require_one_of_tokens({"X", "Y"});

            accept_token("=");

            int sign = 1;
            if (accept_token("+")) {
                sign = 1;
            } else if (accept_token("-")) {
                sign = -1;
            }
            int num = require_int();
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

int part_one(const std::vector<std::string>& lines)
{
    ClawMachineParser parser(lines);
    std::vector<ClawMachine> machines = parser.parse();

    for (const auto& m : machines) {
        std::cout << m << "\n";
    }
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