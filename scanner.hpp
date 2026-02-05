#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "token_type.hpp"
#include "token.hpp"
#include <iostream>

void error(int line, const std::string& message, const std::string& source, int start, int current);

class Scanner {
public:
    Scanner(const std::string& source) : source(source) {}

    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            start = current;   // reset start at beginning of each token
            scanToken();
        }

        // Pop remaining indents
        while (indentLevels.size() > 1) {
            indentLevels.pop_back();
            addToken(TokenType::DEDENT);
        }

        tokens.emplace_back(TokenType::EOF_, "", nullptr, line);
        return tokens;
    }

private:
    const std::string source;
    std::vector<Token> tokens;
    std::vector<int> indentLevels {0};
    int start = 0;
    int current = 0;
    int line = 1;
    bool atLineStart = true;

    bool isAtEnd() const { return current >= source.size(); }
    char advance() { return source[current++]; }
    char peek() const { return isAtEnd() ? '\0' : source[current]; }
    char peekNext() const { return (current + 1 >= source.size()) ? '\0' : source[current + 1]; }
    bool match(char expected) { if (peek() != expected) return false; current++; return true; }

    void scanToken() {
        char c = peek();
        if ((c == 'f' || c == 'F') && peekNext() == '"') {
            advance(); // consume f/F
            advance(); // consume opening "
            fString();
            return;
        }
        if (atLineStart) handleIndentation();
        if (isAtEnd()) return;

        start = current;  // mark start for this token
        c = advance();

        // Helper lambda for two-character operators
        auto twoChar = [&](char expected, TokenType typeIfMatch, TokenType typeSingle) {
            if (!isAtEnd() && peek() == expected) {
                advance();
                addToken(typeIfMatch);
            } else {
                addToken(typeSingle);
            }
        };

        switch (c) {
            // Single-character tokens
            case '(': addToken(TokenType::LEFT_PAREN); break;
            case ')': addToken(TokenType::RIGHT_PAREN); break;
            case '{': addToken(TokenType::LEFT_SQUIGGLE); break;
            case '}': addToken(TokenType::RIGHT_SQUIGGLE); break;
            case '[': addToken(TokenType::LEFT_BRACE); break;
            case ']': addToken(TokenType::RIGHT_BRACE); break;
            case ',': addToken(TokenType::COMMA); break;
            case '.': addToken(TokenType::DOT); break;
            case ';': addToken(TokenType::SEMICOLON); break;
            case ' ':
            case '\r':
            case '\t': break; // skip whitespace
            case '\n': addToken(TokenType::NEWLINE); line++; atLineStart = true; break;

            // Operators (two-char handled consistently)
            case '!': twoChar('=', TokenType::NOT_EQUAL, TokenType::NOT); break;
            case '=': twoChar('=', TokenType::EQUAL_EQUAL, TokenType::EQUAL); break;
            case '<': twoChar('=', TokenType::LESS_EQUAL, TokenType::LESS); break;
            case '>': twoChar('=', TokenType::GREATER_EQUAL, TokenType::GREATER); break;
            case '+':
                if (peek() == '+') { advance(); addToken(TokenType::PLUS_PLUS); }
                else if (peek() == '=') { advance(); addToken(TokenType::PLUS_EQUAL); }
                else addToken(TokenType::PLUS);
                break;
            case '-':
                if (peek() == '-') { advance(); addToken(TokenType::MINUS_MINUS); }
                else if (peek() == '=') { advance(); addToken(TokenType::MINUS_EQUAL); }
                else addToken(TokenType::MINUS);
                break;
            case '*': twoChar('=', TokenType::STAR_EQUAL, TokenType::STAR); break;
            case '/':
                if (peek() == '/') {
                    advance(); // consume second '/'
                    while (!isAtEnd() && peek() != '\n') advance(); // skip comment
                } else twoChar('=', TokenType::SLASH_EQUAL, TokenType::SLASH);
                break;
            case '%': twoChar('=', TokenType::PERCENT_EQUAL, TokenType::PERCENT); break;

            case '"': string(); break;  // string literal

            default:
                if (isdigit(c)) number();
                else if (isalpha(c)) identifier();
                else error(line, "Unexpected character.", source, start, current);
                break;
        }
    }


    void handleIndentation() {
        if (isAtEnd()) return;

        int indent = 0;
        while (peek() == ' ') { advance(); indent++; }

        // Blank line or comment-only line
        if (peek() == '\n' || peek() == '\0' || (peek() == '/' && peekNext() == '/')) {
            atLineStart = false;
            return; // ignore indentation on empty/comment line
        }

        int currentIndent = indentLevels.back();

        if (indent > currentIndent) {
            indentLevels.push_back(indent);
            addToken(TokenType::INDENT);
        } else if (indent < currentIndent) {
            while (indentLevels.size() > 1 && indent < indentLevels.back()) {
                indentLevels.pop_back();
                addToken(TokenType::DEDENT);
            }

            // Only error if indentation is not a known level
            if (indent != indentLevels.back()) {
                error(line, "Bad Indentation.", source, start, current);
            }
        }

        atLineStart = false;
    }

    void fString() {
        std::string literal;
        while (!isAtEnd()) {
            char c = advance();
            if (c == '"') {
                if (!literal.empty()) addToken(TokenType::STRING, literal);
                return;
            } else if (c == '{') {
                if (!literal.empty()) {
                    addToken(TokenType::STRING, literal);
                    literal.clear();
                }

                int exprStart = current;
                int braceDepth = 1;
                while (!isAtEnd() && braceDepth > 0) {
                    char ch = advance();
                    if (ch == '{') braceDepth++;
                    else if (ch == '}') braceDepth--;
                }
                int exprEnd = current - 1;
                std::string exprText = source.substr(exprStart, exprEnd - exprStart);
                addToken(TokenType::FSTRING_EXPR, exprText);
            } else {
                literal += c;
            }
        }
        error(line, "Unterminated f-string.", source, start, current);
    }






    void string() {
        std::string value;
        start++; // skip the opening quote
        while (!isAtEnd()) {
            char c = advance();
            if (c == '"') { // end of string
                addToken(TokenType::STRING, value);
                return;
            }
            if (c == '\\') {
                if (isAtEnd()) { error(line, "Unterminated escape sequence", source, start, current); return; }
                char esc = advance();
                switch (esc) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '"': value += '"'; break;
                    case '\\': value += '\\'; break;
                    default: error(line, "Unknown escape sequence", source, start, current);
                }
            } else {
                value += c;
                if (c == '\n') line++;
            }
        }
        error(line, "Unterminated string", source, start, current);
    }


    void number() {
        while (isdigit(peek())) advance();
        if (peek() == '.' && isdigit(peekNext())) {
            advance();
            while (isdigit(peek())) advance();
        }
        double value = std::stod(source.substr(start, current - start));
        addToken(TokenType::NUMBER, value);
    }

    void identifier() {
        while (isalnum(peek())) advance();
        std::string text = source.substr(start, current - start);

        static const std::unordered_map<std::string, TokenType> keywords = {
            {"and", TokenType::AND}, {"class", TokenType::CLASS}, {"def", TokenType::DEF},
            {"else", TokenType::ELSE}, {"false", TokenType::FALSE}, {"for", TokenType::FOR},
            {"if", TokenType::IF}, {"in", TokenType::IN}, {"input", TokenType::INPUT},
            {"none", TokenType::NONE}, {"or", TokenType::OR}, {"print", TokenType::PRINT},
            {"return", TokenType::RETURN}, {"super", TokenType::SUPER}, {"this", TokenType::THIS},
            {"true", TokenType::TRUE}, {"while", TokenType::WHILE}
        };

        auto it = keywords.find(text);
        if (it != keywords.end()) addToken(it->second);
        else addToken(TokenType::IDENTIFIER, text);
    }

    void addToken(TokenType type) { addToken(type, std::any{}); }
    void addToken(TokenType type, const std::any& literal) {
        std::string lex = source.substr(start, current - start);

        // Check for both STRING and FSTRING_EXPR to override the lexeme
        if ((type == TokenType::STRING || type == TokenType::FSTRING_EXPR) && literal.has_value()) {
            try {
                lex = std::any_cast<std::string>(literal);
            } catch (const std::bad_any_cast& e) {
                // Fallback or handle non-string literals (like numbers) if necessary
            }
        }

        tokens.emplace_back(type, lex, literal, line);
    }

};
