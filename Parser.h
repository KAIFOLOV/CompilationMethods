#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Rule
{
    std::string left;
    std::vector<std::string> right;
    int id;
    std::string operation;
};

using Grammar = std::vector<Rule>;
using RulesByNonterminal = std::unordered_map<std::string, std::vector<const Rule *>>;

class Parser
{
public:
    Parser(const std::string &input,
           const Grammar &grammar,
           const std::unordered_set<std::string> &terminals,
           const std::string &startSymbol);

    bool parse();
    std::vector<int> getDerivation() const;
    std::vector<std::string> getRPN() const;

private:
    std::string _input;
    int _position;
    std::vector<int> _derivation;
    std::vector<std::string> _rpn;

    Grammar _grammar;
    RulesByNonterminal _rulesByLHS;
    std::unordered_set<std::string> _terminals;
    std::string _start;

    bool parseSymbol(const std::string &symbol);
    bool match(char c);
};

#endif // PARSER_H
