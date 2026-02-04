#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "scanner.hpp"



class Axiom{
public:
    inline static bool hadError = false;

    static int main(int argc, char*argv[]){
        if (argc > 2) {
            std::cout << "Usage: axiom [script]\n";
            hadError = true;
        } else if (argc == 2) {
            runFile(argv[1]);
        } else {
            runPrompt();
        }
        hadError = false;
        return 0;
    }

private:
    static void runFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            std::cerr << "Could not open file: " << path << "\n";
            std::exit(65);
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        run(buffer.str());

        if (hadError) std::exit(65);
    }

    static void runPrompt() {
        std::string line;
        for (;;) {
            std::cout << "> ";
            if (line.empty()) continue;
            if (!std::getline(std::cin, line)) {
                std::cout << "\n";
                break;
            }
            run(line);
            hadError = false;
        }
    }

    static void run(const std::string& source) {
        // Placeholder for actual interpretation logic
        std::cout << "Running source:\n" << source << "\n";
        Scanner scanner(source);
        auto tokens = scanner.scanTokens();
        for (const auto& token : tokens) {
            std::cout << token.toString() << "\n";
        }
    }

    static void error(int line, const std::string& message) {
        report(line, "", message);
    }

    static void report(int line, const std::string& where, const std::string& message) {
        std::cerr << "[line " << line << "] Error" << where << ": " << message << "\n";
        hadError = true;
    }
};


int main(int argc, char* argv[]){
    return Axiom::main(argc, argv);
}
