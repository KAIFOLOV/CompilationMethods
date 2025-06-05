#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <stdexcept>

using namespace std;
using mapSets = map<string, set<string>>;

// Constants and global data structures
const vector<string> NONTERMINALS = {"A", "B", "B'", "T", "T'", "M"};
const vector<string> TERMINALS = {"(", "a", "b", "c", "d", "x", "y", "*", "/", "+", "-", ")", "!"};

const map<string, vector<vector<string>>> PRODUCTIONS = {
    {"A", {{"!", "B", "!"}}},
    {"B", {{"B'"}}},
    {"B'", {{"T"}, {"B'", "+", "T"}, {"B'", "-", "T"}}},
    {"T", {{"T'"}}},
    {"T'", {{"M"}, {"T'", "*", "M"}, {"T'", "/", "M"}}},
    {"M", {{"a"}, {"b"}, {"c"}, {"d"}, {"x"}, {"y"}, {"(", "B", ")"}}}
};

vector<string> poliz;

bool contains(const vector<string>& vec, const string& val) {
    return find(vec.begin(), vec.end(), val) != vec.end();
}

pair<mapSets, mapSets> build_sets() {
    mapSets L, R;
    for (const auto& head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto& body : head_pair.second) {
            if (!body.empty()) {
                L[head].insert(body.front());
                R[head].insert(body.back());
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& head_pair : L) {
            string head = head_pair.first;
            for (const string& symbol : head_pair.second) {
                if (contains(NONTERMINALS, symbol) && symbol != head) {
                    for (const string& s : L[symbol]) {
                        if (L[head].insert(s).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
        for (const auto& head_pair : R) {
            string head = head_pair.first;
            for (const string& sym : head_pair.second) {
                if (contains(NONTERMINALS, sym) && sym != head) {
                    for (const string& s : R[sym]) {
                        if (R[head].insert(s).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return {L, R};
}

map<string, map<string, string>> build_precedence_matrix(const mapSets& L, const mapSets& R) {
    map<string, map<string, string>> matrix;

    for (const string& t : TERMINALS) {
        matrix[t];
    }

    for (const auto& head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto& body : head_pair.second) {
            for (size_t i = 0; i < body.size() - 1; ++i) {
                string a = body[i], b = body[i + 1];
                if (contains(TERMINALS, a) && contains(TERMINALS, b)) {
                    matrix[a][b] = "=";
                }
                if (i + 2 < body.size() && contains(TERMINALS, body[i]) &&
                    contains(NONTERMINALS, body[i + 1]) && contains(TERMINALS, body[i + 2])) {
                    matrix[body[i]][body[i + 2]] = "=";
                }
                if (contains(TERMINALS, a) && contains(NONTERMINALS, b)) {
                    for (const string& t : L.at(b)) {
                        matrix[a][t] = "<";
                    }
                }
                if (contains(NONTERMINALS, a) && contains(TERMINALS, b)) {
                    for (const string& t : R.at(a)) {
                        matrix[t][b] = ">";
                    }
                }
            }
        }
    }

    // NOTE: Вручную устанавливаем приоретет между операциями + - и * /
    vector<vector<string>> groups = {{"+", "-"}, {"*", "/"}};
    for (const auto& group : groups) {
        for (const string& a : group) {
            for (const string& b : group) {
                matrix[a][b] = "=";
            }
        }
    }

    for (const string& high : {"*", "/"}) {
        for (const string& low : {"+", "-"}) {
            matrix[high][low] = ">";
            matrix[low][high] = "<";
        }
    }

    return matrix;
}

int main() {
    try {
        auto [L, R] = build_sets();
        auto matrix = build_precedence_matrix(L, R);

        std::cout << "Precedence Matrix:" << std::endl;

        // Шапка таблицы
        std::cout << "    ";
        for (const std::string& t : TERMINALS) {
            std::cout << std::setw(3) << t;
        }
        std::cout << std::endl;

        // Разделительная строка
        std::cout << "----";
        for (const std::string& t : TERMINALS) {
            std::cout << "---";
        }
        std::cout << std::endl;

        // Содержимое матрицы
        for (const std::string& row : TERMINALS) {
            std::cout << std::setw(3) << row << "|"; // Нетерминал в начале строки
            for (const std::string& col : TERMINALS) {
                std::string val = matrix[row][col];
                if (!val.empty()) {
                    std::cout << std::setw(3) << val; // Например: "<", "=", ">"
                } else {
                    std::cout << "   "; // пустая ячейка
                }
            }
            std::cout << std::endl;
        }

    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
