#pragma once
#include <any>
#include <string>
#include "token_type.hpp"

class Token {
public:
    const TokenType type;
    const std::string lexeme;
    const std::any literal;
    const int line;

    Token(TokenType type, const std::string& lexeme, const std::any& literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

        std::string tokenTypeToString(TokenType type) const {
            switch (type) {
                case TokenType::LEFT_PAREN: return "LEFT_PAREN";
                case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
                case TokenType::LEFT_BRACE: return "LEFT_BRACE";
                case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
                case TokenType::COMMA: return "COMMA";
                case TokenType::DOT: return "DOT";
                case TokenType::MINUS: return "MINUS";
                case TokenType::PLUS: return "PLUS";
                case TokenType::SEMICOLON: return "SEMICOLON";
                case TokenType::SLASH: return "SLASH";
                case TokenType::STAR: return "STAR";
                case TokenType::PRECENT: return "PRECENT";
                case TokenType::NOT: return "NOT";
                case TokenType::NOT_EQUAL: return "NOT_EQUAL";
                case TokenType::EQUAL: return "EQUAL";
                case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
                case TokenType::GREATER: return "GREATER";
                case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
                case TokenType::LESS: return "LESS";
                case TokenType::LESS_EQUAL: return "LESS_EQUAL";
                case TokenType::IDENTIFIER: return "IDENTIFIER";
                case TokenType::STRING: return "STRING";
                case TokenType::NUMBER: return "NUMBER";
                case TokenType::AND: return "AND";
                case TokenType::CLASS: return "CLASS";
                case TokenType::DEF: return "DEF";
                case TokenType::ELSE: return "ELSE";
                case TokenType::FALSE: return "FALSE";
                case TokenType::FOR: return "FOR";
                case TokenType::IF: return "IF";
                case TokenType::IN: return "IN";
                case TokenType::INPUT: return "INPUT";
                case TokenType::NONE: return "NONE";
                case TokenType::OR: return "OR";
                case TokenType::PRINT: return "PRINT";
                case TokenType::RETURN: return "RETURN";
                case TokenType::SUPER: return "SUPER";
                case TokenType::THIS: return "THIS";
                case TokenType::TRUE: return "TRUE";
                case TokenType::WHILE: return "WHILE";
                case TokenType::EOF_: return "EOF_";
                default: return "UNKNOWN";
            }
        }

        std::string literalToString() const {
            if (!literal.has_value()) return "nil";

            if (literal.type() == typeid(double))
                return std::to_string(std::any_cast<double>(literal));

            if (literal.type() == typeid(std::string))
                return std::any_cast<std::string>(literal);

            if (literal.type() == typeid(bool))
                return std::any_cast<bool>(literal) ? "true" : "false";

            return "<?>"; // something went sideways
        }

    std::string toString() const {
        return tokenTypeToString(type) + " " + lexeme + " " + literalToString();
    }
};
