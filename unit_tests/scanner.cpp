#include "../scanner.hpp"
#include "../token_type.hpp"
#include "../token.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <any>

std::string tokenTypeToString(TokenType type) {
    static const std::string strings[] = {
        "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "LEFT_SQUIGGLE", "RIGHT_SQUIGGLE", "COMMA", "COLON", "DOT", "MINUS", "MINUS_MINUS", "PLUS", "PLUS_PLUS",
        "SEMICOLON", "SLASH", "STAR", "PERCENT", "PLUS_EQUAL", "MINUS_EQUAL", "SLASH_EQUAL", "STAR_EQUAL", "PERCENT_EQUAL",

        // One or two character
        "NOT", "NOT_EQUAL",
        "EQUAL", "EQUAL_EQUAL",
        "GREATER", "GREATER_EQUAL",
        "LESS", "LESS_EQUAL",

        // Literals
        "IDENTIFIER", "STRING", "NUMBER",

        // Keywords
        "AND", "CLASS", "DEF", "ELSE", "FALSE", "FOR", "IF", "IN", "INPUT", "NONE", "OR", "PRINT", "RETURN", "SUPER", "THIS", "TRUE", "WHILE",

        // Indents and newlines
        "INDENT", "DEDENT", "NEWLINE",

        "FSTRING_EXPR",

        "EOF_"
    };
    return strings[static_cast<int>(type)];
}

void printTokens(const std::vector<Token>& tokens) {
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];

        // Print token index and type
        std::cout << "[" << i << "] " << tokenTypeToString(t.type)
                  << " : \"" << (t.type == TokenType::EOF_ || t.type == TokenType::NEWLINE ? "" : t.lexeme) << "\"";

        // Handle EOF_ explicitly
        if (t.type == TokenType::EOF_) {
            std::cout << " (EOF)";
        }
        else if (t.type == TokenType::NEWLINE) {
            std::cout << " (NEWLINE)";
        }
        // Print literal if it exists
        else if (t.literal.has_value()) {
            std::cout << " (literal: ";
            if (t.literal.type() == typeid(std::string)) {
                std::cout << std::any_cast<std::string>(t.literal);
            } else if (t.literal.type() == typeid(double)) {
                std::cout << std::any_cast<double>(t.literal);
            } else {
                std::cout << "unknown";
            }
            std::cout << ")";
        }

        std::cout << "\n";
    }
}



// Override error function to report during tests
void error(int line, const std::string& message, const std::string& source, int start, int current) {
    std::cerr << "[line " << line << "] Error: " << message << "\n";
    int snippetStart = std::max(0, start - 5);
    int snippetEnd = std::min((int)source.size(), current + 5);
    std::cerr << "  source snippet: \"" << source.substr(snippetStart, snippetEnd - snippetStart) << "\"\n";
    std::cerr << "  start=" << start << ", current=" << current
              << ", peek='" << (current < source.size() ? source[current] : ' ') << "'\n";
}

// Helper to compare tokens
void expect(const Token& token, TokenType type, const std::string& lexeme, int line) {
    if (token.type != type) {
        std::cerr << "[line " << line << "] Expected token type " << static_cast<int>(type)
                  << ", got " << static_cast<int>(token.type) << "\n";
    }
    if (token.lexeme != lexeme) {
        std::cerr << "[line " << line << "] Expected lexeme \"" << lexeme
                  << "\", got \"" << token.lexeme << "\"\n";
    }
    assert(token.type == type);
    assert(token.lexeme == lexeme);
}

// Test 1: basic tokens
static void test_basic_tokens() {
    std::string source = R"(
a = 5
b != 3
c += 2
d /= 4
// hello
)";

    Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    printTokens(tokens);

    // Expected token types in the same order the scanner emits
    std::vector<TokenType> expectedTypes = {
        // TokenType::NEWLINE,           // first empty line
        TokenType::IDENTIFIER,        // a
        TokenType::EQUAL,             // =
        TokenType::NUMBER,            // 5
        TokenType::NEWLINE,           // newline
        TokenType::IDENTIFIER,        // b
        TokenType::NOT_EQUAL,         // !=
        TokenType::NUMBER,            // 3
        TokenType::NEWLINE,
        TokenType::IDENTIFIER,        // c
        TokenType::PLUS_EQUAL,        // +=
        TokenType::NUMBER,            // 2
        TokenType::NEWLINE,
        TokenType::IDENTIFIER,        // d
        TokenType::SLASH_EQUAL,       // /=
        TokenType::NUMBER,            // 4
        TokenType::NEWLINE,
        TokenType::EOF_
    };

    // Lexemes expected
    std::vector<std::string> expectedLexemes = {
        // "\n",
        "a", "=", "5", "\n",
        "b", "!=", "3", "\n",
        "c", "+=", "2", "\n",
        "d", "/=", "4", "\n",
        ""
    };

    if (tokens.size() != expectedTypes.size()) {
        std::cerr << "Token count mismatch: got " << tokens.size()
                  << ", expected " << expectedTypes.size() << "\n";
    }

    for (size_t i = 0; i < tokens.size() && i < expectedTypes.size(); ++i) {
        if (tokens[i].type != expectedTypes[i] || tokens[i].lexeme != expectedLexemes[i]) {
            std::cerr << "[line " << tokens[i].line << "] "
                      << "Expected token type " << static_cast<int>(expectedTypes[i])
                      << ", got " << static_cast<int>(tokens[i].type) << "\n";
            std::cerr << "Expected lexeme \"" << expectedLexemes[i]
                      << "\", got \"" << tokens[i].lexeme << "\"\n";
        }
    }

    assert(tokens.size() == expectedTypes.size());
    for (size_t i = 0; i < tokens.size(); ++i) {
        assert(tokens[i].type == expectedTypes[i]);
        assert(tokens[i].lexeme == expectedLexemes[i]);
    }
    std::cout << "test_basic passed!\n";
}


// Test 2: string literal
void test_string() {
    std::string source = R"("hello\nworld")";
    Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    printTokens(tokens);

    assert(tokens.size() >= 2);
    expect(tokens[0], TokenType::STRING, "hello\nworld", 1);
    expect(tokens[1], TokenType::EOF_, "", 1);
    std::cout << "test_string passed\n";
}

// Test 3: identifier and keywords
void test_identifiers() {
    std::string source = "if foo print return true false none input";
    Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    printTokens(tokens);

    std::vector<TokenType> expectedTypes = {
        TokenType::IF, TokenType::IDENTIFIER, TokenType::PRINT, TokenType::RETURN,
        TokenType::TRUE, TokenType::FALSE, TokenType::NONE, TokenType::INPUT, TokenType::EOF_
    };

    std::vector<std::string> expectedLexemes = {
        "if", "foo", "print", "return", "true", "false", "none", "input", ""
    };

    assert(tokens.size() == expectedTypes.size());

    for (size_t i = 0; i < tokens.size(); i++)
        expect(tokens[i], expectedTypes[i], expectedLexemes[i], tokens[i].line);

    std::cout << "test_identifiers passed\n";
}

void test_fstrings() {
    std::string source = R"(f"Hello {name}, you have {count} {{escaped}} new messages")";
    Scanner scanner(source);
    auto tokens = scanner.scanTokens();

    printTokens(tokens);

    std::vector<TokenType> expectedTypes = {
        TokenType::STRING,
        TokenType::FSTRING_EXPR,
        TokenType::STRING,
        TokenType::FSTRING_EXPR,
        TokenType::STRING,
        TokenType::EOF_
    };

    std::vector<std::string> expectedLexemes = {
        "Hello ",
        "name",
        ", you have ",
        "count",
        " {escaped} new messages",
        ""
    };

    if (tokens.size() != expectedTypes.size()) {
        std::cerr << "Token count mismatch: got " << tokens.size()
                  << ", expected " << expectedTypes.size() << "\n";
    }

    for (size_t i = 0; i < expectedTypes.size(); i++) {
        expect(tokens[i], expectedTypes[i], expectedLexemes[i], 1);
    }
}

int main() {
    test_basic_tokens();
    test_string();
    test_identifiers();
    test_fstrings();

    std::cout << "All tests passed!\n";
    return 0;
}
