#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <memory>
#include <sstream>

using namespace std;

// Struct to represent a class
struct Class {
    string subject;
    string teacher;
    string time; // e.g., "8-9 AM"
    string days; // e.g., "Mon & Wed"
    int time_slot; // Numeric slot for ordering (e.g., 8 for 8-9 AM)

    Class(string s, string t, string tm, string d, int ts)
        : subject(s), teacher(t), time(tm), days(d), time_slot(ts) {}

    string to_string() const {
        return subject + " (" + teacher + ", " + time + ", " + days + ")";
    }
};

// PQ Tree Node Types
enum NodeType { P_NODE, Q_NODE, LEAF };

// PQ Tree Node
struct PQNode {
    NodeType type;
    string label; // For LEAF, the class description
    vector<shared_ptr<PQNode>> children;

    PQNode(NodeType t, string l = "") : type(t), label(l) {}
};

// PQ Tree Class
class PQTree {
private:
    shared_ptr<PQNode> root;

    // Helper to print PQ tree
    void print_tree(shared_ptr<PQNode> node, int depth, stringstream& ss) const {
        string indent(depth * 2, ' ');
        if (node->type == LEAF) {
            ss << indent << "Leaf: " << node->label << "\n";
        } else {
            string type_str = (node->type == P_NODE) ? "P" : "Q";
            ss << indent << "[" << type_str << "]\n";
            for (const auto& child : node->children) {
                print_tree(child, depth + 1, ss);
            }
        }
    }

    // Generate all permutations for a P-node
    void generate_permutations(shared_ptr<PQNode> node, vector<vector<string>>& permutations, vector<string> current) const {
        if (node->type == LEAF) {
            current.push_back(node->label);
            permutations.push_back(current);
            return;
        }
        if (node->type == P_NODE) {
            vector<vector<string>> all_children_labels;
            for (const auto& child : node->children) {
                vector<vector<string>> temp_perms;
                generate_permutations(child, temp_perms, {});
                all_children_labels.insert(all_children_labels.end(), temp_perms.begin(), temp_perms.end());
            }
            // Flatten
            vector<string> flat;
            for (auto& perm : all_children_labels) {
                flat.insert(flat.end(), perm.begin(), perm.end());
            }
            sort(flat.begin(), flat.end());
            do {
                vector<string> temp = current;
                temp.insert(temp.end(), flat.begin(), flat.end());
                permutations.push_back(temp);
            } while (next_permutation(flat.begin(), flat.end()));
        } else { // Q_NODE
            vector<vector<string>> all_children_labels;
            for (const auto& child : node->children) {
                vector<vector<string>> temp_perms;
                generate_permutations(child, temp_perms, {});
                all_children_labels.insert(all_children_labels.end(), temp_perms.begin(), temp_perms.end());
            }

            vector<string> forward, reverse;
            for (auto& perm : all_children_labels) {
                forward.insert(forward.end(), perm.begin(), perm.end());
            }
            reverse = forward;
            std::reverse(reverse.begin(), reverse.end());

            vector<string> temp1 = current;
            temp1.insert(temp1.end(), forward.begin(), forward.end());
            permutations.push_back(temp1);

            if (forward != reverse) {
                vector<string> temp2 = current;
                temp2.insert(temp2.end(), reverse.begin(), reverse.end());
                permutations.push_back(temp2);
            }
        }
    }

public:
    PQTree() {
        root = make_shared<PQNode>(P_NODE);
    }

    void set_root(shared_ptr<PQNode> r) {
        root = r;
    }

    // Add a leaf for a class
    void add_leaf(const Class& c) {
        auto leaf = make_shared<PQNode>(LEAF, c.to_string());
        root->children.push_back(leaf);
    }

    // Build a Q-node for time-ordered classes
    void build_time_ordered_tree(const vector<Class>& classes) {
        vector<Class> sorted_classes = classes;
        sort(sorted_classes.begin(), sorted_classes.end(),
             [](const Class& a, const Class& b) { return a.time_slot < b.time_slot; });

        auto q_node = make_shared<PQNode>(Q_NODE);
        for (const auto& c : sorted_classes) {
            auto leaf = make_shared<PQNode>(LEAF, c.to_string());
            q_node->children.push_back(leaf);
        }
        root = q_node;
    }

    string print() const {
        stringstream ss;
        print_tree(root, 0, ss);
        return ss.str();
    }

    vector<vector<string>> get_frontiers() const {
        vector<vector<string>> permutations;
        generate_permutations(root, permutations, {});
        set<vector<string>> unique(permutations.begin(), permutations.end());
        return vector<vector<string>>(unique.begin(), unique.end());
    }
};

// Conflict check
bool has_conflict(const Class& c1, const Class& c2) {
    if (c1.days != c2.days) return false;
    return c1.time_slot == c2.time_slot;
}

// Constraint filter
bool satisfies_constraints(const Class& c, const string& time_constraint,
                           const string& teacher_constraint, const string& day_constraint) {
    bool time_ok = time_constraint.empty() || c.time == time_constraint;
    bool teacher_ok = teacher_constraint.empty() || c.teacher == teacher_constraint;
    bool day_ok = day_constraint.empty() || c.days == day_constraint;
    return time_ok && teacher_ok && day_ok;
}

// Generate valid schedules
void generate_schedules(const vector<Class>& classes, const string& time_constraint,
                        const string& teacher_constraint, const string& day_constraint,
                        vector<vector<Class>>& valid_schedules) {
    vector<Class> math_classes, comp_classes, eng_classes;
    for (const auto& c : classes) {
        if (c.subject == "Math" && satisfies_constraints(c, time_constraint, teacher_constraint, day_constraint)) {
            math_classes.push_back(c);
        } else if (c.subject == "Computer" && satisfies_constraints(c, time_constraint, teacher_constraint, day_constraint)) {
            comp_classes.push_back(c);
        } else if (c.subject == "English" && satisfies_constraints(c, time_constraint, teacher_constraint, day_constraint)) {
            eng_classes.push_back(c);
        }
    }

    for (const auto& m : math_classes) {
        for (const auto& c : comp_classes) {
            if (has_conflict(m, c)) continue;
            for (const auto& e : eng_classes) {
                if (has_conflict(m, e) || has_conflict(c, e)) continue;
                valid_schedules.push_back({m, c, e});
            }
        }
    }
}

int main() {
    vector<Class> classes = {
        Class("Math", "Miss Maria", "8-9 AM", "Mon & Wed", 8),
        Class("Math", "Sir Qasim", "9-10 AM", "Tue & Thu", 9),
        Class("Computer", "Sir Salman", "11 AM-12 PM", "Mon & Wed", 11),
        Class("Computer", "Miss Maria", "1-2 PM", "Mon & Wed", 13),
        Class("English", "Miss Hamna", "8-9 AM", "Tue & Thu", 8),
        Class("English", "Miss Sara", "2-3 PM", "Mon & Wed", 14)
    };

    // User constraints
    cout << "Enter time constraint (e.g., '8-9 AM' or leave empty): ";
    string time_constraint;
    getline(cin, time_constraint);

    cout << "Enter teacher constraint (e.g., 'Miss Maria' or leave empty): ";
    string teacher_constraint;
    getline(cin, teacher_constraint);

    cout << "Enter day constraint (e.g., 'Mon & Wed' or leave empty): ";
    string day_constraint;
    getline(cin, day_constraint);

    vector<vector<Class>> valid_schedules;
    generate_schedules(classes, time_constraint, teacher_constraint, day_constraint, valid_schedules);

    if (valid_schedules.empty()) {
        cout << "No valid schedules found with the given constraints.\n";
        return 0;
    }

    cout << "\nValid Schedules:\n";
    int schedule_num = 1;
    for (const auto& schedule : valid_schedules) {
        cout << "Schedule " << schedule_num++ << ":\n";
        vector<Class> mon_wed, tue_thu;
        for (const auto& c : schedule) {
            cout << "  " << c.to_string() << "\n";
            if (c.days == "Mon & Wed") {
                mon_wed.push_back(c);
            } else {
                tue_thu.push_back(c);
            }
        }

        if (!mon_wed.empty()) {
            PQTree tree;
            tree.build_time_ordered_tree(mon_wed);
            cout << "  PQ Tree for Mon & Wed:\n" << tree.print();
            cout << "  Valid Orderings for Mon & Wed:\n";
            auto frontiers = tree.get_frontiers();
            for (const auto& frontier : frontiers) {
                cout << "    ";
                for (size_t i = 0; i < frontier.size(); ++i) {
                    cout << frontier[i];
                    if (i < frontier.size() - 1) cout << " -> ";
                }
                cout << "\n";
            }
        }
        if (!tue_thu.empty()) {
            PQTree tree;
            tree.build_time_ordered_tree(tue_thu);
            cout << "  PQ Tree for Tue & Thu:\n" << tree.print();
            cout << "  Valid Orderings for Tue & Thu:\n";
            auto frontiers = tree.get_frontiers();
            for (const auto& frontier : frontiers) {
                cout << "    ";
                for (size_t i = 0; i < frontier.size(); ++i) {
                    cout << frontier[i];
                    if (i < frontier.size() - 1) cout << " -> ";
                }
                cout << "\n";
            }
        }
        cout << "\n";
    }

    return 0;
}