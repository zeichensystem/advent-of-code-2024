#include <array>
#include <unordered_set>
#include "aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/17
  
    Solutions: 
        - Part 1: 7,6,5,3,6,5,7,0,4 (Example: 4,6,3,5,6,3,5,2,1,0)
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

struct ComputerParser;

class Computer 
{
    std::array<int, 3> registers;
    std::vector<int> program;
    int pc = 0; // Instruction pointer.
    std::string _output;
    
    friend ComputerParser;

    int register_read(char name) const
    {
        assert(name >= 'A' && name <= 'C');
        return registers.at(name - 'A');
    }

    void register_write(char name, int val)
    {
        assert(name >= 'A' && name <= 'C');
        registers.at(name - 'A') = val;
    }

    int literal_operand() const
    {
        assert(pc < std::ssize(program) - 1);
        return program.at(pc + 1);
    }

    int combo_operand() const
    {
        assert(pc < std::ssize(program) - 1);
        const int id = program.at(pc + 1);
        if (id <= 3) {
            return id;
        } else if (id >= 4 && id <= 6) {
            char c = 'A' + (id - 4);
            return register_read(c);
        } else {
            throw std::runtime_error("Computer::combo_operand: Combo operand 7 does not appear in vali programs.");
        }
    }

    void dv(char out_reg) { // Used for Opcode 0, 6, and 7
        int num = register_read('A');
        int denom = 1 << combo_operand();
        register_write(out_reg, num / denom);
        pc += 2;
    }

    const std::array<std::function<void(Computer*)>, 8> operations {&Computer::adv, &Computer::bxl, &Computer::bst, &Computer::jnz, &Computer::bxc, &Computer::out, &Computer::bdv, &Computer::cdv};

    void adv() { // Opcode 0
        assert(program.at(pc) == 0);
        dv('A');
    }

    void bxl() { // Opcode 1
        assert(program.at(pc) == 1);
        register_write('B', register_read('B') ^ literal_operand());
        pc += 2;
    }

    void bst() { // Opcode 2
        assert(program.at(pc) == 2);
        register_write('B', combo_operand() % 8);
        pc += 2;
    }

    void jnz() { // Opcode 3
        assert(program.at(pc) == 3);
        if (register_read('A') == 0) {
            pc += 2;
            return;
        } else {
            pc = literal_operand();
        }
    }

    void bxc() { // Opcode 4
        assert(program.at(pc) == 4);
        [[maybe_unused]] int dummy = literal_operand();
        register_write('B', register_read('B') ^ register_read('C'));
        pc += 2;
    }

    void out() { // Opcode 5
        assert(program.at(pc) == 5);
        char c = '0' + (combo_operand() % 8);
        if (_output.size()) {
            _output += ',';
        }
        _output += c;
        pc += 2;
    }

    void bdv() { // Opcode 6
        assert(program.at(pc) == 6);
        dv('B');
    }

    void cdv() { // Opcode 7
        assert(program.at(pc) == 7);
        dv('C');
    }

    public:
    void run_program()
    {
        while (pc < std::ssize(program) - 1) { // (Every instruction takes one operand).
            const int opcode = program.at(pc);
            operations.at(opcode)(this);
        }
    }

    void reset()
    {
        pc = 0; 
        _output = "";
    }

    std::string output() const
    {
        return _output;
    }

    friend std::ostream& operator<<(std::ostream& os, const Computer& c) {
        os << "Register A: " << c.registers.at(0) << "\n"; 
        os << "Register B: " << c.registers.at(1) << "\n";
        os << "Register C: " << c.registers.at(2) << "\n";
        os << "Program: ";
        for (int data : c.program) {
            os << data << ",";
        }
        return os << "Output: " << c.output() << "\n";
    }
};

struct ComputerParser : private aocio::RDParser 
{
    ComputerParser(const std::vector<std::string>& lines) : aocio::RDParser{lines, " \t:,", ":,", false} {};

    Computer parse()
    {
        Computer c;
        registers(c);
        program(c);
        for (int data: c.program) {
            if (data < 0 || data > 7) {
                throw std::invalid_argument("Parser::parse: Program contains number which overflows the computer's 3 bit width.");
            }
        }
        return c;
    }

    private:
    void registers(Computer& c)
    {
        std::unordered_set<std::string> read_registers;  
        for (int i = 0; i < 3; ++i) {
            require_token("Register"); 
            const std::string reg_name = require_one_of_tokens({"A", "B", "C"});
            if (read_registers.contains(reg_name)) {
                throw std::invalid_argument("Register already defined.");
            }
            read_registers.insert(reg_name);
            require_token(":");
            c.registers.at(reg_name.at(0) - 'A') = require_int<int>();
        }
    }
    
    void program(Computer &c)
    {
        require_token("Program"); 
        require_token(":");
        while (true) {
            int data = require_int<int>();
            c.program.push_back(data);
            if (!accept_token(",")) {
                if (peek() != RDParser::EMPTY) {
                    throw std::invalid_argument("Parser::program: Missing comma.");
                }
                break;
            } else if (peek() == RDParser::EMPTY) { // Allow trailing comma.
                break;
            }
        }
    }
};

auto part_one(const std::vector<std::string>& lines)
{
    Computer c = ComputerParser(lines).parse();
    c.run_program();
    return c.output();
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