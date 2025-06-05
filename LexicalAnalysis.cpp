#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <stdexcept>

using namespace std;

// Constants and global data structures
const vector<string> NONTERMINALS = {"A", "B", "T", "M"};
const vector<string> TERMINALS = {"(", "a", "b", "c", "d", "x", "y", "*", "/", "+", "-", ")", "!", "#"};

const map<string, vector<vector<string>>> PRODUCTIONS = {
    {"A", {{"!", "B", "!"}}},
    {"B", {{"T"}, {"B", "+", "T"}, {"B", "-", "T"}}},
    {"T", {{"M"}, {"T", "*", "M"}, {"T", "/", "M"}}},
    {"M", {{"a"}, {"b"}, {"c"}, {"d"}, {"x"}, {"y"}, {"(", "B", ")"}}}
};

const vector<tuple<vector<string>, string, vector<string>, int>> RULES = {
    {{"!", "N", "!"}, "A", {}, 1},
    {{"N", "+", "N"}, "B", {"+"}, 2},
    {{"N", "-", "N"}, "B", {"-"}, 3},
    {{"N", "*", "N"}, "T", {"*"}, 4},
    {{"N", "/", "N"}, "T", {"/"}, 5},
    {{"a"}, "M", {"a"}, 6},
    {{"b"}, "M", {"b"}, 7},
    {{"c"}, "M", {"c"}, 8},
    {{"d"}, "M", {"d"}, 9},
    {{"x"}, "M", {"x"}, 10},
    {{"y"}, "M", {"y"}, 11},
    {{"(", "N", ")"}, "M", {}, 12}
};

vector<string> poliz;

// Precedence Graph class
class PrecedenceGraph {
public:
    vector<string> terminals;
    map<string, vector<string>> graph;

    PrecedenceGraph(const vector<string>& terms) : terminals(terms) {}

    void add_edge(const string& u, const string& v) {
        if (find(graph[u].begin(), graph[u].end(), v) == graph[u].end()) {
            graph[u].push_back(v);
        }
    }

    void build_from_matrix(const map<string, map<string, string>>& matrix) {
        for (const auto& a_pair : matrix) {
            string a = a_pair.first;
            for (const auto& b_pair : matrix.at(a)) {
                string b = b_pair.first;
                string rel = b_pair.second;
                if (rel == ">") {
                    add_edge(a + " F", b + " G");
                } else if (rel == "<") {
                    add_edge(b + " G", a + " F");
                } else if (rel == "=") {
                    for (const string& i : graph[a]) {
                        add_edge(b + " G", i + " G");
                    }
                    for (const string& i : graph[b]) {
                        add_edge(a + " F", i + " F");
                    }
                    for (const string& i : terminals) {
                        string find = i + " G";
                        if (find_in_vector(graph[find], a + " F")) {
                            add_edge(find, b + " G");
                        }
                    }
                    for (const string& i : terminals) {
                        string find = i + " F";
                        if (find_in_vector(graph[find], b + " G")) {
                            add_edge(find, a + " F");
                        }
                    }
                }
            }
        }
    }

private:
    bool find_in_vector(const vector<string>& vec, const string& val) {
        return find(vec.begin(), vec.end(), val) != vec.end();
    }
};

// Helper function to check if a symbol is in a vector
bool is_in(const vector<string>& vec, const string& val) {
    return find(vec.begin(), vec.end(), val) != vec.end();
}

// Build L and R sets
pair<map<string, set<string>>, map<string, set<string>>> build_sets() {
    map<string, set<string>> L, R;
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
            for (const string& sym : head_pair.second) {
                if (is_in(NONTERMINALS, sym) && sym != head) {
                    for (const string& s : L[sym]) {
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
                if (is_in(NONTERMINALS, sym) && sym != head) {
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

// Build Lt and Rt sets
pair<map<string, set<string>>, map<string, set<string>>> build_terminal_sets(
    const map<string, set<string>>& L, const map<string, set<string>>& R) {
    map<string, set<string>> Lt, Rt;
    for (const auto& head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto& body : head_pair.second) {
            if (!body.empty()) {
                for (const string& symbol : body) {
                    if (is_in(TERMINALS, symbol)) {
                        Lt[head].insert(symbol);
                        break;
                    }
                }
                for (auto it = body.rbegin(); it != body.rend(); ++it) {
                    if (is_in(TERMINALS, *it)) {
                        Rt[head].insert(*it);
                        break;
                    }
                }
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& head_pair : Lt) {
            string head = head_pair.first;
            for (const string& sym : L.at(head)) {
                if (is_in(NONTERMINALS, sym) && sym != head) {
                    for (const string& t : Lt[sym]) {
                        if (Lt[head].insert(t).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
        for (const auto& head_pair : Rt) {
            string head = head_pair.first;
            for (const string& sym : R.at(head)) {
                if (is_in(NONTERMINALS, sym) && sym != head) {
                    for (const string& t : Rt[sym]) {
                        if (Rt[head].insert(t).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return {Lt, Rt};
}

// Build precedence matrix
map<string, map<string, string>> build_precedence_matrix(
    const map<string, set<string>>& Lt, const map<string, set<string>>& Rt) {
    map<string, map<string, string>> matrix;

    // Initialize matrix
    for (const string& t : TERMINALS) {
        matrix[t]; // Create empty inner map
    }

    // Base relations from productions
    for (const auto& head_pair : PRODUCTIONS) {
        string head = head_pair.first;
        for (const auto& body : head_pair.second) {
            for (size_t i = 0; i < body.size() - 1; ++i) {
                string a = body[i], b = body[i + 1];
                if (is_in(TERMINALS, a) && is_in(TERMINALS, b)) {
                    matrix[a][b] = "=";
                }
                if (i + 2 < body.size() && is_in(TERMINALS, body[i]) &&
                    is_in(NONTERMINALS, body[i + 1]) && is_in(TERMINALS, body[i + 2])) {
                    matrix[body[i]][body[i + 2]] = "=";
                }
                if (is_in(TERMINALS, a) && is_in(NONTERMINALS, b)) {
                    for (const string& t : Lt.at(b)) {
                        matrix[a][t] = "<";
                    }
                }
                if (is_in(NONTERMINALS, a) && is_in(TERMINALS, b)) {
                    for (const string& t : Rt.at(a)) {
                        matrix[t][b] = ">";
                    }
                }
            }
        }
    }

    // Operator groups with equal precedence
    vector<vector<string>> groups = {{"+", "-"}, {"*", "/"}};
    for (const auto& group : groups) {
        for (const string& a : group) {
            for (const string& b : group) {
                matrix[a][b] = "=";
            }
        }
    }

    // Explicitly set precedence for * and / over + and -
    for (const string& high : {"*", "/"}) {
        for (const string& low : {"+", "-"}) {
            matrix[high][low] = ">"; // Higher precedence
            matrix[low][high] = "<"; // Lower precedence
        }
    }

    // Boundary symbols
    matrix["#"]["("] = "<";
    matrix[")"]["#"] = ">";
    matrix["#"]["!"] = "=";
    matrix["!"]["#"] = "=";

    return matrix;
}

// Check if graph is acyclic
bool is_acyclic(const PrecedenceGraph& graph) {
    set<string> stack;
    vector<string> cycle_nodes;

    function<bool(const string&)> dfs = [&](const string& node) -> bool {
        if (stack.find(node) != stack.end()) {
            cycle_nodes.push_back(node);
            return false;
        }
        stack.insert(node);
        for (const string& neighbor : graph.graph.at(node)) {
            if (!dfs(neighbor)) {
                return false;
            }
        }
        stack.erase(node);
        return true;
    };

    for (const auto& node_pair : graph.graph) {
        if (!dfs(node_pair.first)) {
            return false;
        }
    }
    return true;
}

// Find maximum distance in graph
int find_max_dist(const string& node, const PrecedenceGraph& graph) {
    map<string, int> dist;
    for (const auto& key_pair : graph.graph) {
        dist[key_pair.first] = 0;
    }

    set<string> visited;
    function<void(const string&)> dfs = [&](const string& node) {
        if (visited.find(node) != visited.end()) return;
        visited.insert(node);
        for (const string& neighbor : graph.graph.at(node)) {
            if (visited.find(neighbor) == visited.end()) {
                dist[neighbor] = dist[node] + 1;
                dfs(neighbor);
            }
        }
    };

    dfs(node);
    int max_dist = 0;
    for (const auto& d : dist) {
        max_dist = max(max_dist, d.second);
    }
    return max_dist;
}

// Compute F and G functions
pair<map<string, int>, map<string, int>> compute_fg(const PrecedenceGraph& graph) {
    map<string, int> f, g;
    // Initialize f and g for all terminals with 0
    for (const string& t : TERMINALS) {
        f[t] = 0;
        g[t] = 0;
    }
    for (const auto& node_pair : graph.graph) {
        string node = node_pair.first;
        if (node.find(" ") == string::npos) continue;
        string sym = node.substr(0, node.find(" "));
        string func = node.substr(node.find(" ") + 1);
        if (func == "F") {
            f[sym] = find_max_dist(node, graph);
        } else if (func == "G") {
            g[sym] = find_max_dist(node, graph);
        }
    }
    return {f, g};
}

// Parse input string
vector<int> parse_input(const string& input_str,
                        const map<string, map<string, string>>& matrix,
                        const map<string, int>& f, const map<string, int>& g) {
    // Convert input string to vector of strings
    vector<string> input_vec;
    input_vec.reserve(input_str.size() + 1);
    for (char c : input_str) {
        input_vec.emplace_back(1, c);
    }
    input_vec.push_back("#");

    vector<string> stack = {"#"};
    size_t pos = 0;
    vector<int> steps;

    auto top_term = [&]() -> string {
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            if (is_in(TERMINALS, *it)) {
                return *it;
            }
        }
        return "#";
    };

    while (true) {
        string a = top_term();
        string b = (pos < input_vec.size()) ? input_vec[pos] : "#";

        if (a == "#" && b == "#") {
            break;
        }

        // Debug output
        cout << "Stack: ";
        for (const auto& s : stack) cout << s << " ";
        cout << "| Input pos: " << pos << " | a: " << a << " | b: " << b;
        cout << " | f(a): " << f.at(a) << " | g(b): " << g.at(b);
        cout << " | rel: " << (matrix.at(a).at(b).empty() ? "none" : matrix.at(a).at(b)) << endl;

        // Try to reduce single terminals to M first
        bool reduced = false;
        for (const auto& rule : RULES) {
            const auto& pattern = get<0>(rule);
            const string& nt = get<1>(rule);
            const auto& plz = get<2>(rule);
            int num = get<3>(rule);

            if (pattern.size() == 1 && stack.size() >= 1 && is_in(TERMINALS, stack.back()) && pattern[0] == stack.back()) {
                stack.back() = nt;
                steps.push_back(num);
                poliz.insert(poliz.end(), plz.begin(), plz.end());
                reduced = true;
                cout << "Applied rule " << num << ": " << nt << endl;
                break;
            }
        }

        if (!reduced) {
            string rel = matrix.at(a).at(b);
            if (rel.empty()) {
                throw runtime_error("No relation between " + a + " and " + b);
            }

            if (rel == "<" || rel == "=") {
                stack.push_back(b);
                pos++;
                cout << "Shifted: " << b << endl;
            } else {
                // Try to reduce operator productions
                bool found = false;
                for (const auto& rule : RULES) {
                    const auto& pattern = get<0>(rule);
                    const string& nt = get<1>(rule);
                    const auto& plz = get<2>(rule);
                    int num = get<3>(rule);

                    if (stack.size() >= pattern.size()) {
                        vector<string> window(stack.end() - pattern.size(), stack.end());
                        bool match = true;
                        for (size_t i = 0; i < pattern.size(); ++i) {
                            if (pattern[i] == "N" && is_in(NONTERMINALS, window[i])) {
                                continue;
                            } else if (pattern[i] != window[i]) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            // Check if we should reduce based on the next input symbol
                            string next_b = (pos + 1 < input_vec.size()) ? input_vec[pos + 1] : "#";
                            bool should_reduce = true;
                            if (pattern.size() == 3 && pattern[1] == "+" && matrix.at(a).at(next_b) == "<") {
                                should_reduce = false; // Defer reduction for + if next symbol has higher precedence
                            }
                            if (should_reduce) {
                                stack.resize(stack.size() - pattern.size());
                                stack.push_back(nt);
                                steps.push_back(num);
                                poliz.insert(poliz.end(), plz.begin(), plz.end());
                                found = true;
                                cout << "Applied rule " << num << ": " << nt << endl;
                                break;
                            }
                        }
                    }
                }
                if (!found) {
                    throw runtime_error("No matching rule found after trying reduction");
                }
            }
        }
    }

    steps.push_back(1);
    return steps;
}

// Main function
int main() {
    try {
        auto [L, R] = build_sets();
        auto [Lt, Rt] = build_terminal_sets(L, R);
        auto matrix = build_precedence_matrix(Lt, Rt);

        // Print precedence matrix
        cout << "Precedence Matrix:" << endl;
        cout << "   ";
        for (const string& t : TERMINALS) {
            cout << t << " ";
        }
        cout << endl;
        for (const string& row : TERMINALS) {
            cout << row << " |";
            for (const string& col : TERMINALS) {
                string val = matrix[row][col];
                cout << (val.empty() ? "   " : " " + val + " ");
            }
            cout << endl;
        }

        // Build and check graph
        PrecedenceGraph graph(TERMINALS);
        graph.build_from_matrix(matrix);

        if (!is_acyclic(graph)) {
            throw runtime_error("Граф содержит циклы");
        }

        auto [f, g] = compute_fg(graph);

        // Print f and g for debugging
        cout << "\nF values:" << endl;
        for (const auto& p : f) {
            cout << p.first << ": " << p.second << endl;
        }
        cout << "\nG values:" << endl;
        for (const auto& p : g) {
            cout << p.first << ": " << p.second << endl;
        }

        // Parse example input
        string input_str = "!a*b+c+(a+x)!";
        vector<int> steps = parse_input(input_str, matrix, f, g);
        cout << "\nParsing steps: ";
        for (int step : steps) {
            cout << step << " ";
        }
        cout << "\nPOLIZ: ";
        for (const string& p : poliz) {
            cout << p << " ";
        }
        cout << endl;

    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
