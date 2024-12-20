#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <limits>
#include <cassert>
#include <optional>
#include <concepts>
#include <utility>

#ifndef AOC_DAY_NAME
#define AOC_DAY_NAME "Undefined AOC_DAY_NAME"
#endif

namespace aocio 
{

inline bool file_getlines(std::string_view fname, std::vector<std::string>& lines)
{
    std::ifstream file {fname};
    if (!file) {
        std::cerr << "Error: Cannot open file '" << fname << "'\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return true;
}

inline void remove_leading_empty_lines(std::vector<std::string>& lines)
{
    auto line = lines.begin(); 
    while (line != lines.end()) {
        std::size_t idx = line->find_first_not_of(" \t", 0); 
        if (idx == std::string::npos) {
            lines.erase(line);
            line = lines.begin();
        } else {
            assert(line->size());
            break;
        }
    }
}

inline void remove_trailing_empty_lines(std::vector<std::string>& lines)
{
    auto line = lines.rbegin(); 
    while (line != lines.rend()) {
        std::size_t idx = line->find_first_not_of(" \t", 0); 
        if (idx == std::string::npos) {
            lines.erase(std::next(line).base()); // cf. https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator [1]
            line = lines.rbegin();
        } else {
            assert(line->size());
            break;
        }
    }
    // [1] last retrieved 2024-06-25
}

inline void line_tokenise(const std::string& line, const std::string& delims, const std::string& preserved_delims, std::vector<std::string>& tokens)
{
    for (char d : preserved_delims) {
        if (delims.find(d) == std::string::npos) {
            throw std::invalid_argument("Preserved delim not in delims");
        }
    }

    std::string::size_type start_pos = 0;
    while (start_pos < line.size()) {
        auto token_end_pos = line.find_first_of(delims, start_pos); 
        if (token_end_pos == std::string::npos) {
            token_end_pos = line.size();
        }
        std::string token = line.substr(start_pos, token_end_pos - start_pos);
        if (token.size()) {
            tokens.push_back(token);
        }
        
        if (token_end_pos < line.size() && preserved_delims.size() && preserved_delims.find(line.at(token_end_pos)) != std::string::npos) {
            tokens.push_back(std::string{line.at(token_end_pos)});
        }

        start_pos = token_end_pos + 1;
    }
}

template<class ConversionFn = std::function<std::string(void)>>
inline auto line_tokenise(const std::string& line, const std::string& delims, const std::string& preserved_delims, ConversionFn conversion_fn = []() {return std::string{""};}) -> std::vector<decltype(ConversionFn(""))>
requires std::invocable<ConversionFn&, const std::string&> || std::invocable<ConversionFn&, std::string_view>
{
    std::vector<std::string> tokens;
    line_tokenise(line, delims, preserved_delims, tokens);
    std::vector<decltype(conversion_fn(""))> result; 
    std::transform(tokens.cbegin(), tokens.cend(), std::back_inserter(result), conversion_fn);
    return result;
}

template<class ConversionFn = std::function<std::string(void)>>
inline auto line_tokenise(const std::string& line, const std::string& delims, const std::string& preserved_delims, ConversionFn conversion_fn = []() {return std::string{""};}) -> std::vector<std::string>
requires std::invocable<ConversionFn&>
{
    std::vector<std::string> tokens;
    line_tokenise(line, delims, preserved_delims, tokens);
    return tokens;
}

static inline std::string str_without_whitespace(std::string_view str) 
{
    std::string result; 
    auto is_ws = [](char c) -> bool { return c == ' ' || c == '\t'; }; 
    std::remove_copy_if(str.cbegin(), str.cend(), std::back_inserter(result), is_ws);
    return result;
}

static inline std::string& str_remove_whitespace(std::string& str) 
{
    auto is_ws = [](char c) -> bool { return c == ' ' || c == '\t'; }; 
    auto no_space_end = std::remove_if(str.begin(), str.end(), is_ws);
    str.erase(no_space_end, str.end());
    return str;
}

inline std::string str_tolower_cpy(std::string s) { // cf. https://en.cppreference.com/w/cpp/string/byte/tolower (last retrieved 2024-12-01)
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);});
    return s;
}
inline std::string str_toupper_cpy(std::string s) { // cf. see above
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::toupper(c);});
    return s;
}

inline std::string& str_make_lower(std::string& s) { // In-place variant; cf. see above
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);});
    return s;
}
inline std::string& str_make_upper(std::string& s) { // In-place variant; cf. see above
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::toupper(c);});
    return s;
}

static inline std::optional<int> parse_num(const std::string &str, bool strict = false)
{
    size_t num_read = 0; 
    int n; 
    try {
        n = std::stoi(str, &num_read); 
    } catch (const std::invalid_argument& err) {
        return {};
    } catch (const std::out_of_range& err) {
        std::cerr << "parse_num: the converted value would fall out of the range of int";
        return {};
    }
    
    if (strict && num_read != str.size()) {
        return {};
    }
    if (num_read == 0) {
        return {};
    }
    return n; 
} 

static inline std::optional<int64_t> parse_num_i64(const std::string& str, bool strict = false)
{
    size_t num_read = 0; 
    int64_t n; 
    try {
        n = std::stoll(str, &num_read); 
    } catch (const std::invalid_argument& err) {
        return {};
    } catch (const std::out_of_range& err) {
        std::cerr << "parse_num_i64: the converted value would fall out of the range of int64_t";
        return {};
    }
    
    if (strict && num_read != str.size()) {
        return {};
    }
    if (num_read == 0) {
        return {};
    }
    return n; 
} 

static inline std::optional<int> parse_digit(char c)
{
    int digit = static_cast<int>(c) - '0'; 
    if (digit >= 0 && digit <= 9) {
        return digit; 
    } else {
        return {};
    }
}

template <typename IntT = int>
static inline std::optional<IntT> parse_hex(std::string_view str)
{
    if (str.size() == 0) {
        return {};
    }

    int prefix = 0; 
    while (str.at(prefix) == ' ' || str.at(prefix) == '\t') { // Remove leading whitespace
        ++prefix; 
    }
    str = str.substr(prefix, str.size());

    if (str.size() >= 2) { // Handle # and 0x prefixes
        if (str.at(0) == '#') {
            str = str.substr(1, str.size());
        } else if (str.size() >= 3) {
            if (str.at(0) == '0' && str.at(1) == 'x') {
                str = str.substr(2, str.size());
            }
        }
    } 
 
    int end_idx = std::ssize(str) - 1; 
    while (str.at(end_idx) == ' ' || str.at(end_idx) == '\t') { // Remove trailing whitespace. 
        end_idx -= 1; 
    }

    IntT res = 0; 
    IntT fac = 1; 
    for (int i = end_idx; i >= 0; --i, fac *= 16) {
        char sym = str.at(i); 
        IntT digit = 0; 
        if (sym >= '0' && sym <= '9') {
            digit = aocio::parse_digit(sym).value(); 
        } else {
            digit = 10 + (sym - 'a'); 
            if (digit < 10 || digit > 15) { // Try uppercase. 
                digit = 10 + (sym - 'A'); 
            }
            if (digit < 10 || digit > 15) {
                return {};
            }
        }
        res += digit * fac;
    }
    return res; 
}

inline void print_day() 
{
    std::string day_name {AOC_DAY_NAME};
    
    if (day_name.size()) {
        day_name[0] = std::toupper(day_name[0]);
    }

    std::string debug_release;
    #ifdef NDEBUG
    debug_release = "Release";
    #else
    debug_release = "Debug";
    #endif

    std::cout << day_name << " (" << debug_release << ")\n";
}

enum class IOStatus {UNDEFINED = 0, INPUT_SUCCESS = 1, INPUT_HELP = 2, OUTPUT_VERBOSE = 4}; 

constexpr IOStatus operator|(IOStatus a, IOStatus b)
{
    return static_cast<IOStatus>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr IOStatus operator&(IOStatus a, IOStatus b)
{
    return static_cast<IOStatus>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

constexpr bool iostat_has_flag(IOStatus status, IOStatus flag)
{
    return (status & flag) == flag;
}

inline IOStatus handle_input(int argc, char* argv[], std::vector<std::string>& lines)
{
    const auto print_help = []() -> void {
        #ifdef NDEBUG
            #define POSTFIX ""
        #else 
            #define POSTFIX "_dbg"
        #endif
        std::cerr << "Usage: " AOC_DAY_NAME POSTFIX " [-help] puzzle_input [-v]\n" << "\t-v: use verbose output (optional)\n" << "\t-help: print this help, ignore the rest, and quit (optional)\n" << "\tpuzzle_input: your puzzle input file (optional/ignored if -help is used)\n"; 
        #undef POSTFIX
    };

    IOStatus result = IOStatus::UNDEFINED;

    std::string_view fname = ""; 

    if (argc <= 1) { // Failure: Program run without arguments.
        std::cerr << "Error: No puzzle input file.\n";
        print_help();
        return IOStatus::UNDEFINED;
    } 
    else if (argc >= 2) { // Program run with at least one argument.
        std::vector<std::string_view> args {};
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        assert(args.size() >= 1);
        
        if (args.front() == "-help") {  // Success: program -help ...
            print_help();
            return IOStatus::INPUT_SUCCESS | IOStatus::INPUT_HELP;
        } else if (args.front() == "-v") { // program -v ...
            result = result | IOStatus::OUTPUT_VERBOSE;
            if (args.size() < 2 || args.at(1) == "-v" || args.at(1) == "-help") { // Failure: program -v or program -v -v ... or program -v -help ...
                std::cerr << "Error: No puzzle input file.\n";
                print_help();
                return IOStatus::UNDEFINED;
            } else { // program -v foo ...
                fname = args.at(1);
            }
        } else { // program foo ...
            fname = args.front(); 
            if (args.size() > 1 && args.at(1) == "-v") { // program foo -v ...
                result = result | IOStatus::OUTPUT_VERBOSE;
            }
        }

        if (iostat_has_flag(result, IOStatus::OUTPUT_VERBOSE)) {
            assert(fname != "");
            std::cerr << "Using input '" << fname << "'\n"; 
        }
    }

    if (!aocio::file_getlines(fname, lines)) { // Failure.
        print_help();
        return IOStatus::UNDEFINED;
    }    

    aocio::remove_leading_empty_lines(lines);
    aocio::remove_trailing_empty_lines(lines);
    if (!lines.size()) { // Failure.
        std::cerr << "Error: Input file '" << fname << "' is empty (or contains only whitespace).\n";
        print_help();
        return IOStatus::UNDEFINED;
    }

    // Success.
    assert(!iostat_has_flag(result, IOStatus::INPUT_HELP));
    return result | IOStatus::INPUT_SUCCESS;
}

class RDParser 
{
    // Recursive descent parser (kinda), cf. https://en.wikipedia.org/wiki/Recursive_descent_parser (last retrieved 2024-12-15)
    const std::vector<std::string>& lines;
    const std::string tokenize_delims; 
    const std::string preserved_delims;

    std::size_t line = 0; 
    std::string::size_type col = 0;
    std::string_view _current_token = EMPTY;
    bool _is_end = false, _is_new_line = false;

    void advance_to_newline() 
    {
        ++line;
        col = 0; 
    }

    void handle_eol() 
    {
        if (preserve_newlines) {
            _current_token = NEWLINE;
            advance_to_newline();
        } else {
            advance_to_newline();
            next_token();
        }
    }

    public:
    bool preserve_newlines;
    static inline const std::string NEWLINE = "\n";
    static inline const std::string EMPTY = "";
    
    RDParser(const std::vector<std::string>& lines, const std::string& tokenize_delims = " \t", const std::string& preserved_delims = "", bool preserve_newlines = true) :lines{lines}, tokenize_delims{tokenize_delims}, preserved_delims{preserved_delims}, preserve_newlines{preserve_newlines}
    {
        for (char preserved : preserved_delims) {
            if (tokenize_delims.find(preserved) == std::string::npos) {
                throw std::invalid_argument("RDParser::RDParser: preserved delim not in delims.");
            }
        }

        if (lines.size() == 0) {
            _is_end = true;
        }

        next_token();
    }

    void reset()
    {
        line = 0; 
        col = 0;
        _current_token = EMPTY;
        _is_end = lines.size() == 0;
        next_token();
    }

    void next_token() 
    {
        assert(line <= lines.size());

        _is_new_line = (col == 0);

        if (_is_end || line == lines.size()) {
            _is_end = true;
            _current_token = EMPTY;
            return;
        }

        if (col >= lines.at(line).size()) { // Reached end of line. 
            return handle_eol();
        } 
  
        std::string::size_type start_col = lines.at(line).find_first_not_of(tokenize_delims, col); 

        if (preserved_delims.size()) {
            std::string::size_type preserved_delim_idx = lines.at(line).find_first_of(preserved_delims, col); 
            if (preserved_delim_idx < start_col) {
                _current_token = std::string_view{lines.at(line)}.substr(preserved_delim_idx, 1);
                col = preserved_delim_idx + 1;
                return;
            }
        }

        if (start_col == std::string::npos) { // Reached end of line.
            return handle_eol();
        } 
        
        std::string::size_type end_col = lines.at(line).find_first_of(tokenize_delims, start_col);
        if (preserved_delims.size()) {
            std::string::size_type preserved_delim_idx = lines.at(line).find_first_of(preserved_delims, start_col);
            if (preserved_delim_idx < end_col) {
                end_col = preserved_delim_idx;
            }
        }

        assert(end_col > start_col);
        _current_token = std::string_view{lines.at(line)}.substr(start_col, end_col - start_col);
        assert(_current_token != "");
        col = end_col != std::string::npos ? end_col : lines.at(line).size();
    }

    bool accept_token(std::string_view sym) 
    {
        if (_current_token == sym) {
            next_token();
            return true;
        } 
        return false;
    }

    bool require_token(std::string_view sym)
    {
        if (accept_token(sym)) {
            return true;
        } else {
            const std::string offending_line = line < lines.size() ? lines.at(line) : "";
            const std::string info = "on line " + std::to_string(line + 1) + " (col " + std::to_string(col - current_token().size()) + "):\n'" + offending_line + "'";
            throw std::runtime_error("Parser::require_token: Required token\n'" + std::string{sym} + "'\ndoes not match actual token\n'" + std::string{current_token()} + "'\n" + info);
        }
    }

    std::string require_one_of_tokens(std::initializer_list<std::string_view> syms)
    {
        for (const auto& sym : syms) {
            if (accept_token(sym)) {
                return std::string{sym};
            }
        }
        const std::string offending_line = line < lines.size() ? lines.at(line) : "";
        const std::string info = "on line " + std::to_string(line + 1) + " (col " + std::to_string(col - current_token().size()) + "):\n'" + offending_line + "'";
        throw std::runtime_error("Parser::require_one_of_tokens: Required tokens do not match actual token\n'" + std::string{current_token()} + "'\n" + info);
    }

    template<class IntT> std::optional<IntT> accept_int(); 
    template<>
    std::optional<int> accept_int<int>()
    {
        const auto num = aocio::parse_num(std::string{_current_token}, true); 
        if (num.has_value()) {
            next_token();
        }
        return num;
    }
    template<>
    std::optional<int64_t> accept_int<int64_t>()
    {
        const auto num = aocio::parse_num_i64(std::string{_current_token}, true); 
        if (num.has_value()) {
            next_token();
        }
        return num;
    }

    template<class IntT> IntT require_int();
    template<>
    int require_int<int>()
    {
        const auto num = aocio::parse_num(std::string{_current_token}, true); 
        if (num.has_value()) {
            next_token();
            return num.value();
        }
        const std::string offending_line = line < lines.size() ? lines.at(line) : "";
        const std::string info = "on line " + std::to_string(line + 1) + " (col " + std::to_string(col - current_token().size()) + "):\n'" + offending_line + "'";
        throw std::runtime_error("Parser::require_int: Required int token does not match actual token\n'" + std::string{current_token()} + "'\n" + info);
    }
    template<>
    int64_t require_int<int64_t>()
    {
        const auto num = aocio::parse_num_i64(std::string{_current_token}, true); 
        if (num.has_value()) {
            next_token();
            return num.value();
        }
        const std::string offending_line = line < lines.size() ? lines.at(line) : "";
        const std::string info = "on line " + std::to_string(line + 1) + " (col " + std::to_string(col - current_token().size()) + "):\n'" + offending_line + "'";
        throw std::runtime_error("Parser::require_int: Required int token does not match actual token\n'" + std::string{current_token()} + "'\n" + info);
    }

    bool accept_newline() 
    {
        if (_current_token == NEWLINE) {
            next_token();
            return true;
        } 
        return false;
    }

    bool require_newline()
    {
        if (accept_token(NEWLINE)) {
            return true;
        } else {
            const std::string offending_line = line < lines.size() ? lines.at(line) : "";
            const std::string info = "on line " + std::to_string(line + 1) + " (col " + std::to_string(col - current_token().size()) + "):\n'" + offending_line + "'";
            throw std::runtime_error("Parser::require_newline: Required token\n'" + std::string{NEWLINE} + "'\ndoes not match actual token\n'" + std::string{current_token()} + "'\n" + info);
        }
    }

    std::string_view current_token() const
    {
        return _current_token;
    }

    std::string_view peek() const
    {
        return current_token();
    }

    bool is_end() const {
        return _is_end;
    }
    bool is_newline() const {
        return _is_new_line;
    }
};

}
