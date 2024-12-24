#include <array>
#include <unordered_set>
#include <algorithm>
#include "aoclib/aocio.hpp"

/*
    Problem: https://adventofcode.com/2024/day/17
  
    Solutions: 
        - Part 1: "7,6,5,3,6,5,7,0,4" (Example: "4,6,3,5,6,3,5,2,1,0")
        - Part 2: 190615597431823 
    Notes:  
        - Part 1: This was fun!
        - Part 2: Wrote a "compiler" which generates c++ code for a given program, and then analysed the generated code, so this solution is 
                  specific to my puzzle input and not general. 
                  (Realise only 3 bits at a time are being processed of reg_a, and reg_b only depends on the lowest 3 bits
                   of reg_a each iteration, and reg_c only depending on reg_a shifted by reg_b bits.)
                   Work backwards from the last expected output and generate the bits using backtracking, keeping track
                   of the "touched" bits using a bitmask.)
*/

typedef uint64_t register_int_t;
struct ComputerParser;

class Computer 
{
    friend ComputerParser;

    std::array<register_int_t, 3> registers;
    const decltype(registers) initial_registers;
    std::vector<uint8_t> program;
    int pc = 0; // Instruction pointer.
    std::vector<uint8_t> outputs;

    struct Instruction 
    {
        const std::function<void(Computer&)> run; 
        const std::function<std::string(const Computer& c, int program_idx)> to_str; 
        const int opcode = 0;
        const int num_operands = 1;
        static const int to_str_align_width = 34;
        static std::string& str_align_right(std::string& s)
        {
            while (s.size() < Instruction::to_str_align_width) s += " ";
            return s;
        }
        void operator()(Computer& c) const 
        {
            assert(opcode == c.program.at(c.pc)); 
            run(c);
        }
    };

    std::array<const Instruction*, 8> operations {
        &adv, &bxl, &bst, &jnz, 
        &bxc, &out, &bdv, &cdv
    }; 

    public:

    Computer(const std::array<register_int_t, 3>& registers, const std::vector<uint8_t>& program) : registers{registers}, initial_registers{registers}, program{program}
    {
        decltype(operations) ordered_ops;
        std::fill(ordered_ops.begin(), ordered_ops.end(), nullptr);

        for (size_t i = 0; i < operations.size(); ++i) { // Make sure a given operation's index in the operations array will correspond to its opcode.
            if (const Instruction* op = operations.at(i); op != nullptr) {
                if (op->opcode < 0 || op->opcode >= std::ssize(ordered_ops)) {
                    throw std::runtime_error("Computer::Computer: Invalid opcode in .operations.");
                } else if (ordered_ops.at(op->opcode) != nullptr) {
                    throw std::runtime_error("Computer::Computer: Duplicate opcode in .operations.");
                } else {
                    ordered_ops.at(op->opcode) = op;
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
            const char c = 'A' + (id - 4);
            const std::string reg_name = std::string{static_cast<char>(std::tolower(int{c}))}; 
            return "reg_" + reg_name;
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
        register_int_t denom = register_int_t{1} << combo_operand();
        register_write(out_reg, num / denom);
        pc += 2;
    }

    std::string dv_str(char out_reg, int program_counter) const
    {
        const std::string reg_name = std::string{static_cast<char>(std::tolower(int{out_reg}))}; 
        const std::string instr_name = "(" + reg_name + "dv)";
        std::string str = "reg_" + reg_name + " = reg_a / (1ull << " + combo_operand_str(program_counter + 1) + ");";
        return Instruction::str_align_right(str) + "// " + instr_name;
    } 

    const Instruction adv = {
        .opcode = 0,
        .run = [](Computer& c) {
            c.dv('A'); 
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('A', program_counter);
        }
    };
    const Instruction bdv {
        .opcode = 6,
        .run = [](Computer& c) {
            c.dv('B');
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('B', program_counter); 
        }
    }; 
    const Instruction cdv {
        .opcode = 7,
        .run = [](Computer& c) {
            c.dv('C');
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            return c.dv_str('C', program_counter); 
        }
    };

    const Instruction bxl = {
        .opcode = 1,
        .run = [](Computer& c) {
            c.register_write('B', c.register_read('B') ^ c.literal_operand());
            c.pc += 2; 
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            std::string str = "reg_b = reg_b ^ " + c.literal_operand_str(program_counter + 1) + ";";
            return Instruction::str_align_right(str) + "// (bxl)";
        }
    };

    const Instruction bst = {
        .opcode = 2,
        .run = [](Computer& c) {
            c.register_write('B', c.combo_operand() % 8);
            c.pc += 2;
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            std::string str = "reg_b = " + c.combo_operand_str(program_counter + 1) + " % 8;";
            return Instruction::str_align_right(str) + "// (bst)";
        }
    };

    const Instruction jnz = {
        .opcode = 3,
        .run = [](Computer& c) {
            if (c.register_read('A') != 0) {
                c.pc = c.literal_operand();
            } else {
                c.pc += 2;
            }
        }, 
        .to_str = [](const Computer& c, int program_counter) {
            std::string str = "if (reg_a != 0) goto pc_" + c.literal_operand_str(program_counter + 1) + ";";
            return Instruction::str_align_right(str) + "// (jnz)";
        }
    };

    const Instruction bxc {
        .opcode = 4,
        .run = [](Computer& c) {
            [[maybe_unused]] uint8_t dummy_operand = c.literal_operand();
            c.register_write('B', c.register_read('B') ^ c.register_read('C'));
            c.pc += 2;
        }, 
        .to_str = [](const Computer& c, int program_counter) -> std::string {
            std::string str = "reg_b = reg_b ^ reg_c;";
            return Instruction::str_align_right(str) + "// (bxc)";
        }
    };

    const Instruction out {
        .opcode = 5, 
        .run = [](Computer& c) {
            c.outputs.push_back(c.combo_operand() % 8);
            c.pc += 2;
        },
        .to_str = [](const Computer& c, int program_counter) {
            std::string str = "outputs.push_back(" + c.combo_operand_str(program_counter + 1) + " % 8);";
            return Instruction::str_align_right(str) + "// (out)";
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

        if (const Instruction* instr = operations.at(opcode); instr != nullptr) {
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

    std::string program_compile() const
    {
        std::string compiled = "void run_program(std::vector<uint8_t>& outputs"; 
        compiled += ", register_int_t reg_a = " + std::to_string(register_read('A')); 
        compiled += ", register_int_t reg_b = " + std::to_string(register_read('B'));
        compiled += ", register_int_t reg_c = " + std::to_string(register_read('C'));
        compiled += ")\n{\n";
        
        for (size_t i = 0; i < program.size(); ++i) {
            const auto opcode = program.at(i);
            if (opcode >= operations.size()) {
                compiled += "    INVALID_INSTRUCTION\n";
            } else if (const Instruction* op = operations.at(opcode); op != nullptr) {
                compiled += "    pc_" + std::to_string(i) + ":\t" + op->to_str(*this, i) + "\n";
                i += op->num_operands;
            } else {
                compiled += "    INVALID_INSTRUCTION\n";
            }
        }
        return compiled + "}\n";
    }

    std::string program_to_str() const
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
        return os << "Output: " << c.outputs_to_str() << "\n";
    }
};

struct ComputerParser : private aocio::RDParser 
{
    ComputerParser(const std::vector<std::string>& lines) : aocio::RDParser{lines, " \t:,", ":,", false} {};

    Computer parse()
    {
        auto regs = registers();
        auto prog = program();
        return Computer(regs, prog);
    }

    std::array<register_int_t, 3> registers()
    {
        std::array<register_int_t, 3> regs;
        std::unordered_set<std::string> read_registers;  
        for (int i = 0; i < 3; ++i) {
            require_token("Register"); 
            const std::string reg_name = require_one_of_tokens({"A", "B", "C"});
            if (read_registers.contains(reg_name)) {
                throw std::invalid_argument("ComputerParser::registers: Register already defined.");
            }
            read_registers.insert(reg_name);
            require_token(":");
            const register_int_t reg_val = require_int<register_int_t>();
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

std::string part_one(const std::vector<std::string>& lines)
{
    Computer c = ComputerParser(lines).parse();
    c.run_program();
    return c.outputs_to_str();
}

// void run_program(std::vector<uint8_t>& outputs, register_int_t reg_a = 27334280, register_int_t reg_b = 0, register_int_t reg_c = 0)
// {
//     pc_0:    reg_b = reg_a % 8;                // (bst)
//     pc_2:	reg_b = reg_b ^ 2;                // (bxl)
//     pc_4:	reg_c = reg_a / (1ull << reg_b);  // (cdv)
//     pc_6:	reg_a = reg_a / (1ull << 3);      // (adv)
//     pc_8:	reg_b = reg_b ^ 7;                // (bxl)
//     pc_10:	reg_b = reg_b ^ reg_c;            // (bxc)
//     pc_12:	outputs.push_back(reg_b % 8);     // (out)
//     pc_14:	if (reg_a != 0) goto pc_0;        // (jnz)
// }

std::optional<register_int_t> find_quine_reg_a(const std::vector<uint8_t>& expected_outputs, register_int_t reg_a = 0, register_int_t touched_bits_mask = 0, ptrdiff_t out_i = -0xdead)
{
    std::optional<register_int_t> result = {};
    
    if (out_i == -0xdead) {
        out_i = std::ssize(expected_outputs) - 1;
    } 
    if (out_i < 0) {
        return reg_a;
    } 

    reg_a <<= 3;
    touched_bits_mask <<= 3;

    const register_int_t saved_touched_bits_mask = touched_bits_mask;
    const register_int_t saved_reg_a = reg_a;

    // Find the next bits of reg_a.
    for (uint64_t lo_bits = 0; lo_bits <= 7; ++lo_bits) { 
        reg_a = saved_reg_a;
        touched_bits_mask = saved_touched_bits_mask;

        reg_a |= (lo_bits & ~touched_bits_mask);
        touched_bits_mask |= 7;

        register_int_t reg_b = reg_a % 8;
        reg_b = reg_b ^ 2;

        for (uint64_t hi_bits = 0; hi_bits <= 7; ++hi_bits) {
            register_int_t new_reg_a = reg_a | ((hi_bits << reg_b) & ~touched_bits_mask);
            register_int_t new_touched_bits_mask = touched_bits_mask | (7 << reg_b);

            register_int_t reg_c = (new_reg_a / (1ull << reg_b));
            uint8_t out = ((reg_b ^ 7) ^ reg_c) % 8; 

            if (out == expected_outputs.at(out_i)) {
                if (out_i == std::ssize(expected_outputs) - 1 && (new_reg_a >> 3) != 0) {
                    continue;
                }
                if (auto res = find_quine_reg_a(expected_outputs, new_reg_a, new_touched_bits_mask, out_i - 1); res) {
                    return res.value();
                } 
            }
        }
    }
    return result;
}

auto part_two(const std::vector<std::string>& lines)
{
    Computer c = ComputerParser(lines).parse();
    const std::vector<uint8_t> expected_outputs = c.copy_program();
    const std::string expected_outputs_str = c.program_to_str();

    std::cout << "Machine code:\n" << c.program_to_str() << "\n\n";
    std::cout << "Compiled to C++:\n" <<  c.program_compile() << "\n";

    const auto quine_reg_a = find_quine_reg_a(expected_outputs);
    if (!quine_reg_a.has_value()) {
        std::cerr << "No solution found to generate a quine.\n";
        return register_int_t(0);
    } 
    
    c.register_write('A', quine_reg_a.value());
    c.run_program();
    if (c.get_outputs() != expected_outputs) {
        std::cerr << "Found 'solution' of reg_a = " << quine_reg_a.value() << " is wrong: ";
        std::cerr << "Expected output is " << expected_outputs_str << ", actual output is " << c.outputs_to_str() << "\n";
        return register_int_t(0);
    }

    return quine_reg_a.value();
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