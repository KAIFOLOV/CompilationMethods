#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <QMap>

using namespace std;
using mapSets = map<string, set<string>>;

vector<string> POLIZ;
vector<int> PRODUCTION_NUMBERS;

const vector<string> NONTERMINALS = { "A", "B", "B'", "T", "T'", "M" };
const vector<string> TERMINALS = { "(", "a", "b", "c", "d", "x", "y",
                                   "*", "/", "+", "-", ")", "!", "#" };

const map<string, vector<vector<string>>> PRODUCTIONS = {
    { "A", { { "!", "B", "!" } } },
    { "B", { { "B'" } } },
    { "B'", { { "T" }, { "B'", "+", "T" }, { "B'", "-", "T" } } },
    { "T", { { "T'" } } },
    { "T'", { { "M" }, { "T'", "*", "M" }, { "T'", "/", "M" } } },
    { "M", { { "a" }, { "b" }, { "c" }, { "d" }, { "x" }, { "y" }, { "(", "B", ")" } } }
};

bool contains(const vector<string> &vec, const string &val)
{
    return find(vec.begin(), vec.end(), val) != vec.end();
}

pair<mapSets, mapSets> buildSets()
{
    mapSets L, R;
    for (const auto &head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto &body : head_pair.second) {
            if (!body.empty()) {
                L[head].insert(body.front());
                R[head].insert(body.back());
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &head_pair : L) {
            string head = head_pair.first;
            for (const string &symbol : head_pair.second) {
                if (contains(NONTERMINALS, symbol) && symbol != head) {
                    for (const string &s : L[symbol]) {
                        if (L[head].insert(s).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
        for (const auto &head_pair : R) {
            string head = head_pair.first;
            for (const string &sym : head_pair.second) {
                if (contains(NONTERMINALS, sym) && sym != head) {
                    for (const string &s : R[sym]) {
                        if (R[head].insert(s).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return { L, R };
}

QMap<string, QMap<string, string>> buildPrecedenceMatrix(const mapSets &L, const mapSets &R)
{
    QMap<string, QMap<string, string>> matrix;

    for (const string &t : TERMINALS) {
        matrix[t];
    }

    for (const auto &head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto &body : head_pair.second) {
            for (size_t i = 0; i < body.size() - 1; ++i) {
                string a = body[i], b = body[i + 1];
                if ((contains(NONTERMINALS, a) && contains(TERMINALS, b))
                    || (contains(TERMINALS, a) && contains(NONTERMINALS, b))) {
                    matrix[a][b] = "=";
                }
                if (i + 2 < body.size() && contains(TERMINALS, body[i])
                    && contains(NONTERMINALS, body[i + 1]) && contains(TERMINALS, body[i + 2])) {
                    matrix[body[i]][body[i + 2]] = "=";
                }
                if (contains(TERMINALS, a) && contains(NONTERMINALS, b)) {
                    for (const string &t : L.at(b)) {
                        matrix[a][t] = "<";
                    }
                }
                if (contains(NONTERMINALS, a) && contains(TERMINALS, b)) {
                    for (const string &t : R.at(a)) {
                        matrix[t][b] = ">";
                    }
                }
            }
        }
    }

    vector<vector<string>> groups = { { "+", "-" }, { "*", "/" } };
    for (const auto &group : groups) {
        for (const string &a : group) {
            for (const string &b : group) {
                matrix[a][b] = "=";
            }
        }
    }

    for (const string &high : { "*", "/" }) {
        for (const string &low : { "+", "-" }) {
            matrix[high][low] = ">";
            matrix[low][high] = "<";
        }
    }

    matrix["#"]["("] = "<";
    matrix[")"]["#"] = ">";
    matrix["#"]["!"] = "=";
    matrix["!"]["#"] = "=";

    return matrix;
}

bool isTerminal(const string &string)
{
    return contains(TERMINALS, string) || string == "#";
}

string getLastTerminal(const vector<string> &stack)
{
    for (int i = (int)stack.size() - 1; i >= 0; --i) {
        return stack[i];
    }
    throw runtime_error("No terminal symbol found in stack");
}

bool tryReduce(vector<string> &stack)
{
    map<string, map<vector<string>, int>> production_to_number = {
        { "A", { { { "!", "B", "!" }, 1 } } },
        { "B", { { { "B'" }, 2 } } },
        { "B'", { { { "T" }, 3 }, { { "B'", "+", "T" }, 4 }, { { "B'", "-", "T" }, 5 } } },
        { "T", { { { "T'" }, 6 } } },
        { "T'", { { { "M" }, 7 }, { { "T'", "*", "M" }, 8 }, { { "T'", "/", "M" }, 9 } } },
        { "M",
          { { { "a" }, 10 },
            { { "b" }, 11 },
            { { "c" }, 12 },
            { { "d" }, 13 },
            { { "x" }, 14 },
            { { "y" }, 15 },
            { { "(", "B", ")" }, 16 } } }
    };

    vector<tuple<int, string, vector<string>, int>> candidates;
    for (const auto &[head, bodies] : PRODUCTIONS) {
        for (const auto &body : bodies) {
            int n = (int)body.size();
            if ((int)stack.size() >= n) {
                bool match = true;
                for (int i = 0; i < n; ++i) {
                    if (stack[stack.size() - n + i] != body[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    int rule_number = production_to_number[head][body];
                    candidates.emplace_back(n, head, body, rule_number);
                }
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
        return std::get<0>(a) > std::get<0>(b);
    });

    if (!candidates.empty()) {
        auto [n, head, body, rule_number] = candidates.front();

        cout << "Reduce: ";
        for (int i = stack.size() - n; i < (int)stack.size(); ++i) cout << stack[i] << " ";
        cout << "-> " << head << endl;
        cout << "Rule number: " << rule_number << endl;

        stack.erase(stack.end() - n, stack.end());
        stack.push_back(head);
        PRODUCTION_NUMBERS.push_back(rule_number);

        if (head == "B'") {
            if (n == 3 && (body[1] == "+" || body[1] == "-")) {
                POLIZ.push_back(body[1]);
                cout << "POLIZ push operator: " << body[1] << endl;
            }
        } else if (head == "T'") {
            if (n == 3 && (body[1] == "*" || body[1] == "/")) {
                POLIZ.push_back(body[1]);
                cout << "POLIZ push operator: " << body[1] << endl;
            }
        } else if (head == "M") {
            if (n == 1 && isTerminal(body[0]) && body[0] != "(" && body[0] != ")") {
                POLIZ.push_back(body[0]);
                cout << "POLIZ push operand: " << body[0] << endl;
            }
        }

        cout << "Stack after reduce: ";
        for (const auto &s : stack) cout << s << " ";
        cout << endl;

        return true;
    }

    return false;
}

bool translate(const vector<string> &input, const QMap<string, QMap<string, string>> &matrix)
{
    vector<string> stack = { "#" };
    int pos = 0;
    POLIZ.clear();
    PRODUCTION_NUMBERS.clear();

    while (true) {
        string a = getLastTerminal(stack);
        string b = (pos < (int)input.size()) ? input[pos] : "#";

        string relation;
        if (matrix.count(a) && matrix.value(a).count(b)) {
            relation = matrix.value(a).value(b);
        } else {
            relation = "";
        }

        cout << "Stack: ";
        for (const auto &s : stack) cout << s << " ";
        cout << " | Next input symbol: " << b << endl;
        cout << "Relation between '" << a << "' and '" << b
             << "': " << (relation.empty() ? "(none)" : relation) << endl;

        if (a == "#" && b == "#") {
            if (stack.size() == 5 && stack[0] == "#" && stack[1] == "!" && stack[2] == "B"
                && stack[3] == "!" && stack[4] == "#") {
                stack.clear();
                stack.push_back("#");
                stack.push_back("A");
                PRODUCTION_NUMBERS.push_back(1);
                return true;
            } else {
                return false;
            }
        }

        if (relation == "<" || relation == "=") {
            cout << "Action: Shift '" << b << "'" << endl;
            stack.push_back(b);
            pos++;
        } else if (relation == ">") {
            cout << "Action: Reduce" << endl;
            if (!tryReduce(stack)) {
                cerr << "Error: reduction failed." << endl;
                return false;
            }
        } else {
            cerr << "Error: invalid precedence relation between '" << a << "' and '" << b << "'"
                 << endl;
            return false;
        }
    }
}

int main()
{
    try {
        auto [L, R] = buildSets();
        auto matrix = buildPrecedenceMatrix(L, R);

        std::cout << "Precedence Matrix:" << std::endl;

        std::cout << "    ";
        for (const auto key : matrix.keys()) {
            std::cout << std::setw(3) << key;
        }
        std::cout << std::endl;
        std::cout << "--------------------------------------------" << std::endl;

        // Содержимое матрицы
        for (const auto key : matrix.keys()) {
            std::cout << std::setw(3) << key << "|";
            for (const std::string &col : TERMINALS) {
                std::string val = matrix[key][col];
                if (!val.empty()) {
                    std::cout << std::setw(3) << val;
                } else {
                    std::cout << "   ";
                }
            }
            std::cout << std::endl;
        }

        vector<string> input = { "!", "(", "a", "+", "b", ")", "*", "c", "!" };
        std::cout << "\nStarting translation..." << std::endl;
        if (translate(input, matrix)) {
            std::cout << "\nProduction numbers: ";
            for (const auto &num : PRODUCTION_NUMBERS) {
                std::cout << num << " ";
            }

            std::cout << "Parsing succeeded.\nPOLIZ: ";
            for (const auto &tok : POLIZ) {
                std::cout << tok << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "Parsing failed." << std::endl;
        }

    } catch (const exception &e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
