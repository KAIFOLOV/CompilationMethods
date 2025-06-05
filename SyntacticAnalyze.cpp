#include "Parser.h"
#include <iostream>
#include <unordered_set>

int main()
{
    Grammar grammar = {
        { "A", { "!", "B", "!" }, 1, "" },
        { "B", { "T", "+", "B" }, 3, "+" },
        { "B", { "T" }, 2, "" },
        { "T", { "M", "*", "T" }, 5, "*" },
        { "T", { "M" }, 4, "" },
        { "M", { "a" }, 6, "a" },
        { "M", { "b" }, 7, "b" },
        { "M", { "(", "B", ")" }, 8, "" }
    };

    std::unordered_set<std::string> TERMINALS = { "!", "+", "*", "(", ")", "a", "b" };

    std::vector<std::string> testСases = { "!a+b!",         "!a*b!",
                                            "!(a+b)*(b+a)!", "!b*a+a*b!",
                                            "!(a+b)*a+b*a!", "!(a+b*a)*(b*b+a*(a+b+a))!",
                                            "!a+*b!",        "a+b*a+b",
                                            "a!b",           "!a(b+a()!" };

    for (const auto &input : testСases) {
        std::cout << "Parse string: " << input << std::endl;
        Parser parser(input, grammar, TERMINALS, "A");
        if (parser.parse()) {
            std::cout << "Derivation: ";
            for (int id : parser.getDerivation()) {
                std::cout << id << " ";
            }
            std::cout << std::endl;

            std::cout << "RPN: ";
            for (const auto &tok : parser.getRPN()) std::cout << tok << " ";
            std::cout << std::endl;
        } else {
            std::cout << "error" << std::endl;
        }
    }

    return 0;
}
