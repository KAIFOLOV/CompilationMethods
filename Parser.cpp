#include "Parser.h"

Parser::Parser(const std::string &input,
               const Grammar &grammar,
               const std::unordered_set<std::string> &terminals,
               const std::string &startSymbol) :
    _input(input),
    _position(0),
    _grammar(grammar),
    _terminals(terminals),
    _start(startSymbol)
{
    for (const Rule &rule : _grammar) {
        _rulesByLHS[rule.left].push_back(&rule);
    }
}

bool Parser::match(char c)
{
    if (_position < _input.size() && _input[_position] == c) {
        ++_position;
        return true;
    }
    return false;
}

bool Parser::parseSymbol(const std::string &symbol)
{
    // Терминал
    if (_terminals.count(symbol)) {
        return match(symbol[0]);
    }

    // Нетерминал
    auto it = _rulesByLHS.find(symbol);
    if (it == _rulesByLHS.end())
        return false;

    int savedPosition = _position;
    auto savedDerivation = _derivation;

    for (const Rule *rule : it->second) {
        _position = savedPosition;
        _derivation = savedDerivation;

        bool success = true;
        _derivation.push_back(rule->id);

        for (const std::string &rhsSymbol : rule->right) {
            if (!parseSymbol(rhsSymbol)) {
                success = false;
                break;
            }
        }

        if (success) {
            return true;
        }
    }

    _position = savedPosition;
    _derivation = savedDerivation;
    return false;
}

bool Parser::parse()
{
    _position = 0;
    _derivation.clear();
    return parseSymbol(_start) && _position == _input.length();
}

std::vector<int> Parser::getDerivation() const
{
    return _derivation;
}
