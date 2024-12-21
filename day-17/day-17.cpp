#include <array>
#include <unordered_set>
#include <algorithm>
#include "aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/17
  
    Solutions: 
        - Part 1: "7,6,5,3,6,5,7,0,4" (Example: "4,6,3,5,6,3,5,2,1,0")
        - Part 2: 
    Notes:  
        - Part 1: 
        - Part 2:
*/

template<class RegisterIntT>
struct ComputerParser;

template<class RegisterIntT>
class Computer 
{
    friend ComputerParser<RegisterIntT>;

    std::vector<uint8_t> program;
    std::array<RegisterIntT, 3> registers;
    const decltype(registers) initial_registers;
    int pc = 0; // Instruction pointer.
    std::vector<uint8_t> outputs;

    template<class Computer = Computer<RegisterIntT>>
    struct Instruction {
        const std::function<void(Computer&)> run; 
        const std::function<std::string(const Computer& c, int program_idx)> to_str; 
        const int opcode = 0;
        const int num_operands = 1;
        void operator()(Computer& c) const 
        {
            assert(opcode == c.program.at(c.pc)); 
            run(c);
        }
    };

    std::array<const Instruction<Computer>*, 8> operations {
        &adv, &bxl, &bst, &jnz, 
        &bxc, &out, &bdv, &cdv
    }; 

    public:

    typedef RegisterIntT register_int_t;

    Computer(const std::array<register_int_t, 3>& registers, const std::vector<uint8_t>& program) : program{program}, registers{registers}, initial_registers{registers}
    {
        decltype(operations) ordered_ops;
        std::fill(ordered_ops.begin(), ordered_ops.end(), nullptr);
        for (int i = 0; i < std::ssize(operations); ++i) { // Make sure a given operation's index in the operations array will correspond to its opcode.
            if (const auto& op_ptr = operations.at(i); op_ptr != nullptr) {
                if (op_ptr->opcode < 0 || op_ptr->opcode >= std::ssize(ordered_ops)) {
                    throw std::runtime_error("Computer::Computer: Invalid opcode in .operations.");
                } else if (ordered_ops.at(op_ptr->opcode) != nullptr) {
                    throw std::runtime_error("Computer::Computer: Duplicate opcode in .operations.");
                } else {
                    ordered_ops.at(op_ptr->opcode) = op_ptr;
                }
            }
        }
        operations = ordered_ops;
    }

    register_int_t register_read(char name) const
    {
        assert(name >= 'A' && name <= 'C');
        return registers.at(name - 'A');
    }

    void register_write(char name, register_int_t val)
    {
        assert(name >= 'A' && name <= 'C');
        registers.at(name - 'A') = val;
    }

    private:

    uint8_t literal_operand() const
    {
        assert(pc < std::ssize(program) - 1);
        return program.at(pc + 1);
    }

    register_int_t combo_operand() const
    {
        assert(pc < std::ssize(program) - 1);
        const auto id = program.at(pc + 1);
        if (id <= 3) {
            return static_cast<register_int_t>(id);
        } else if (id >= 4 && id <= 6) {
            char c = 'A' + (id - 4);
            return register_read(c);
        } else {
            throw std::runtime_error("Computer::combo_operand: Combo operand 7 does not appear in valid programs.");
        }
    }

    std::string combo_operand_str(size_t operand_idx) const {
        if (operand_idx >= program.size()) {
            return "MISSING_COMBO_OPERAND";
        }
        const auto id = program.at(operand_idx);
        if (id <= 3) {
            return std::to_string(int{id});
        } else if (id >= 4 && id <= 6) {
            char c = 'A' + (id - 4);
            return "reg[" + std::string{c} + "]";
        } else {
            return "INVALID_COMBO_OPERAND";
        }
    }

    std::string literal_operand_str(size_t operand_idx) const 
    {
        if (operand_idx >= program.size()) {
            return "MISSING_LITERAL_OPERAND";
        }
        return std::to_string(int{program.at(operand_idx)});
    }

    void dv(char out_reg) { // Used for Opcode 0, 6, and 7
        register_int_t num = register_read('A');
        register_int_t denom = 1 << combo_operand();
        register_write(out_reg, num / denom);
        pc += 2;
    }

    std::string dv_str(char out_reg, int program_counter) const
    {
        std::string name = "(" + std::string{static_cast<char>(std::tolower(int{out_reg}))} + "dv) ";
        return name + "reg[" + std::string{out_reg} + "] = reg[A] / (1 << " + combo_operand_str(program_counter + 1) + ")"; 
    } 

    const Instruction<Computer> adv = {
        .opcode = 0,
        .run = [](Computer& c) {
            c.dv('A'); 
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('A', program_counter);
        }
    };
    const Instruction<Computer> bdv {
        .opcode = 6,
        .run = [](Computer& c) {
            c.dv('B');
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('B', program_counter); 
        }
    }; 
    const Instruction<Computer> cdv {
        .opcode = 7,
        .run = [](Computer& c) {
            c.dv('C');
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('C', program_counter); 
        }
    };

    const Instruction<Computer> bxl = {
        .opcode = 1,
        .run = [](Computer& c) {
            c.register_write('B', c.register_read('B') ^ c.literal_operand());
            c.pc += 2; 
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return "(bxl) reg[B] = reg[B] xor " + c.literal_operand_str(program_counter + 1);
        }
    };

    const Instruction<Computer> bst = {
        .opcode = 2,
        .run = [](Computer& c) {
            c.register_write('B', c.combo_operand() % 8);
            c.pc += 2;
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return "(bst) reg[B] = " + c.combo_operand_str(program_counter + 1) + " mod 8";
        }
    };

    const Instruction<Computer> jnz = {
        .opcode = 3,
        .run = [](Computer& c) {
            if (c.register_read('A') != 0) {
                c.pc = c.literal_operand();
            } else {
                c.pc += 2;
            }
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return "(jnz) if (reg[A] != 0) goto " + c.literal_operand_str(program_counter + 1);
        }
    };

    const Instruction<Computer> bxc {
        .opcode = 4,
        .run = [](Computer& c) {
            [[maybe_unused]] uint8_t dummy_operand = c.literal_operand();
            c.register_write('B', c.register_read('B') ^ c.register_read('C'));
            c.pc += 2;
        }, 
        .to_str = [](const Computer& c, int program_counter) -> std::string {
            return "(bxc) reg[B] = reg[B] xor reg[C]";
        }
    };

    const Instruction<Computer> out {
        .opcode = 5, 
        .run = [](Computer& c) {
            c.outputs.push_back(c.combo_operand() % 8);
            c.pc += 2;
        },
        .to_str = [](const Computer& c, int program_counter) {
            return "(out) print(" + c.combo_operand_str(program_counter + 1) + " mod 8)";
        }
    };

    public:

    bool run_next_instruction()
    {
        if (pc >= std::ssize(program)) {
            return false;
        }

        const auto opcode = program.at(pc); 
        assert(opcode < operations.size());

        if (const Instruction<Computer>* instr = operations.at(opcode); instr != nullptr) {
            if (pc + instr->num_operands >= std::ssize(program)) {
                throw std::runtime_error("Computer::run_next_instruction: Missing operand.");
            }
            (*instr)(*this);
            return true;
        } 

        throw std::runtime_error("Computer::run_next_instruction: Undefined Instruction.");
    }

    void run_program()
    {
        while (run_next_instruction());
    }

    void reset()
    {
        outputs.clear();
        registers = initial_registers; 
        pc = 0; 
    }

    std::string program_disassembly() const
    {
        std::string assembly;
        for (size_t i = 0; i < program.size(); ++i) {
            const auto opcode = program.at(i);
            if (const auto& op_ptr = operations.at(opcode); op_ptr != nullptr) {
                assembly += std::to_string(i) + ":\t" + op_ptr->to_str(*this, i) + "\n";
                i += op_ptr->num_operands;
            } else {
                assembly += "INVALID_INSTRUCTION\n";
            }
        }
        return assembly;
    }

    std::string program_to_str()
    {
        std::string s; 
        for (size_t i = 0; i < program.size(); ++i) {
            char c = '0' + program.at(i);
            assert(c >= '0' && c <= '7');
            s += c;
            if (i != program.size() - 1) {
                s += ',';
            }
        }
        return s;
    }

    std::string outputs_to_str() const
    {
        std::string result = "";
        for (int i = 0; i < std::ssize(outputs); ++i) {
            result += std::to_string(outputs.at(i));
            if (i != std::ssize(outputs) - 1) {
                result += ",";
            }
        }
        return result;
    }

    const decltype(outputs)& get_outputs() const {
        return outputs;
    }

    const decltype(program)& get_program() const {
        return program;
    }

    decltype(program) copy_program() const {
        return program;
    }

    decltype(registers) copy_registers() const
    {
        return registers;
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

template<class RegisterIntT>
struct ComputerParser : private aocio::RDParser 
{
    ComputerParser(const std::vector<std::string>& lines) : aocio::RDParser{lines, " \t:,", ":,", false} {};

    Computer<RegisterIntT> parse()
    {
        return Computer<RegisterIntT>(registers(), program());
    }

    std::array<RegisterIntT, 3> registers()
    {
        std::array<RegisterIntT, 3> regs;
        std::unordered_set<std::string> read_registers;  
        for (int i = 0; i < 3; ++i) {
            require_token("Register"); 
            const std::string reg_name = require_one_of_tokens({"A", "B", "C"});
            if (read_registers.contains(reg_name)) {
                throw std::invalid_argument("ComputerParser::registers: Register already defined.");
            }
            read_registers.insert(reg_name);
            require_token(":");
            const RegisterIntT reg_val = require_int<RegisterIntT>();
            regs.at(reg_name.at(0) - 'A') = reg_val;
        }
        return regs;
    }
    
    std::vector<uint8_t> program()
    {
        std::vector<uint8_t> prog;
        require_token("Program"); 
        require_token(":");
        while (true) {
            int data = require_int<int>();
            if (data < 0 || data > 7) {
                throw std::invalid_argument("ComputerParser::program: Program instruction/operand would overflow 3 bits.");
            }
            prog.push_back(static_cast<uint8_t>(data));
            if (!accept_token(",")) {
                if (peek() != RDParser::EMPTY) {
                    throw std::invalid_argument("ComputerParser::program: Missing comma.");
                }
                break;
            } else if (peek() == RDParser::EMPTY) { // Allow trailing comma.
                break;
            }
        }
        return prog;
    }
};

auto part_one(const std::vector<std::string>& lines)
{
    Computer c = ComputerParser<uint64_t>(lines).parse();
    c.run_program();
    return c.outputs_to_str();
}

/*
Assembly:
0:	(bst) reg[B] = reg[A] mod 8
2:	(bxl) reg[B] = reg[B] xor 2
4:	(cdv) reg[C] = reg[A] / (1 << reg[B])
6:	(adv) reg[A] = reg[A] / (1 << 3)
8:	(bxl) reg[B] = reg[B] xor 7
10:	(bxc) reg[B] = reg[B] xor reg[C]
12:	(out) print(reg[B] mod 8)
14:	(jnz) if (reg[A] != 0) goto 0
*/

uint64_t run_program() // "Compiled" by hand...
{ 
    constexpr std::array<uint8_t, 16> expected_out = {2,4,1,2,7,5,0,3,1,7,4,1,5,5,3,0};
    constexpr uint64_t upper_bound = 1ull << (3 * expected_out.size()); // exclusive.
    constexpr uint64_t lower_bound = upper_bound / 8ull; // inclusive

    for (uint64_t reg_a_val = lower_bound; reg_a_val < upper_bound; ++reg_a_val) {        
        uint64_t reg_a = reg_a_val, reg_b = 0, reg_c = 0;
        size_t i = 0; 
        prog_start:
        reg_b = reg_a % 8;              //  0: (bst) reg[B] = reg[A] mod 8
        reg_b = reg_b ^ 2;              //  2: (bxl) reg[B] = reg[B] xor 2
        reg_c = reg_a / (1 << reg_b);   //  4: (cdv) reg[C] = reg[A] / (1 << reg[B])
        reg_a = reg_a / (1 << 3);       //  6: (adv) reg[A] = reg[A] / (1 << 3)
        reg_b = reg_b ^ 7;              //  8: (bxl) reg[B] = reg[B] xor 7
        reg_b = reg_b ^ reg_c;          // 10: (bxc) reg[B] = reg[B] xor reg[C]
        uint8_t out = reg_b % 8;        // 12: (out) print(reg[B] mod 8)
        if (i >= expected_out.size() || out != expected_out.at(i)) { 
            assert(i <= expected_out.size());
            continue; // Try next initial value for register a.
        }
        if (reg_a != 0) {                // 14:	(jnz) if (reg[A] != 0) goto 0
            ++i;
            goto prog_start;
        }
        return reg_a;
    }

    std::cerr << "Did not find value for register a\n";
    return 0; 
}

auto part_two(const std::vector<std::string>& lines)
{
    Computer c = ComputerParser<uint64_t>(lines).parse();
    const auto expected_outputs = c.copy_program();

    std::cout << "Machine code:\n" << c.program_to_str() << "\n";
    std::cout << "Assembly:\n" <<  c.program_disassembly();

    uint64_t reg_a_upper_bound = 1ull << (3 * c.get_program().size()); // exclusive.
    uint64_t reg_a_lower_bound = reg_a_upper_bound / 8ull; // inclusive
    std::cout << "Trying " << (reg_a_upper_bound - reg_a_lower_bound) << " different values for register a...\n";

    return run_program();

    // for (uint64_t reg_a = reg_a_lower_bound; reg_a < reg_a_upper_bound; ++reg_a) {
    //     c.reset();
    //     c.register_write('A', reg_a);
    //     c.run_program();
    //     assert(expected_outputs.size() == c.get_outputs().size());
    //     bool fail = false;
    //     for (size_t i = 0; i < expected_outputs.size(); ++i) {
    //         if (c.get_outputs().at(i) != expected_outputs.at(i)) {
    //             fail = true;
    //             break;
    //         }
    //     }
    //     if (!fail) {
    //         return reg_a;
    //     }
    // }
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