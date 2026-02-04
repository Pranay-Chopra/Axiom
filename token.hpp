#include <any>
#include <string>

class Token {
public:
    const TokenType type;
    const std::string lexeme;
    const std::any literal;
    const int line;

    Token(TokenType type, const std::string& lexeme, const std::any& literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}
};
