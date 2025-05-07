#include "PQTree.hpp"
#include <algorithm>
#include <queue>
#include <map>
#include <random>
#include <sstream>
#include <set>
#include <functional>

// PQNode implementation
PQNode::PQNode(NodeType type, const std::string& label)
    : type(type), label(label), x(0), y(0) {}

NodeType PQNode::getType() const {
    return type;
}

std::string PQNode::getLabel() const {
    return label;
}

void PQNode::setLabel(const std::string& label) {
    this->label = label;
}

void PQNode::addChild(std::shared_ptr<PQNode> child) {
    children.push_back(child);
}

const std::vector<std::shared_ptr<PQNode>>& PQNode::getChildren() const {
    return children;
}

int PQNode::getX() const {
    return x;
}

int PQNode::getY() const {
    return y;
}

void PQNode::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

// PQTree implementation
PQTree::PQTree() : root(nullptr) {}

void PQTree::setRoot(std::shared_ptr<PQNode> node) {
    root = node;
}

std::shared_ptr<PQNode> PQTree::getRoot() const {
    return root;
}

std::shared_ptr<PQNode> PQTree::createLeaf(const std::string& label) {
    return std::make_shared<PQNode>(NodeType::LEAF, label);
}

std::shared_ptr<PQNode> PQTree::createPNode(const std::string& label) {
    return std::make_shared<PQNode>(NodeType::P_NODE, label);
}

std::shared_ptr<PQNode> PQTree::createQNode(const std::string& label) {
    return std::make_shared<PQNode>(NodeType::Q_NODE, label);
}

// Build a time-ordered tree from a set of sections
void PQTree::buildTimeOrderedTree(const std::vector<std::shared_ptr<Section>>& sections) {
    // Sort sections by their time slot
    std::vector<std::shared_ptr<Section>> sortedSections = sections;
    std::sort(sortedSections.begin(), sortedSections.end(),
        [](const std::shared_ptr<Section>& a, const std::shared_ptr<Section>& b) {
            return (a->getTimeSlot()->getDay() < b->getTimeSlot()->getDay()) || 
                   (a->getTimeSlot()->getDay() == b->getTimeSlot()->getDay() && 
                    a->getTimeSlot()->getStartHour() < b->getTimeSlot()->getStartHour());
        });
    
    // Create a Q-node as root
    auto qNode = createQNode("TimeOrdered");
    for (const auto& section : sortedSections) {
        std::string label = section->getCourse()->getCode() + " (" + 
                           section->getTeacher()->getName() + ", " +
                           section->getTimeSlot()->toString() + ")";
        auto leaf = createLeaf(label);
        qNode->addChild(leaf);
    }
    root = qNode;
}

// Print the tree structure to a string
std::string PQTree::print() const {
    if (!root) return "Empty Tree";
    
    std::stringstream ss;
    printTree(root, 0, ss);
    return ss.str();
}

// Helper to print the tree
void PQTree::printTree(std::shared_ptr<PQNode> node, int depth, std::stringstream& ss) const {
    std::string indent(depth * 2, ' ');
    if (node->getType() == NodeType::LEAF) {
        ss << indent << "Leaf: " << node->getLabel() << "\n";
    } else {
        std::string typeStr = (node->getType() == NodeType::P_NODE) ? "P" : "Q";
        ss << indent << "[" << typeStr << "] " << node->getLabel() << "\n";
        for (const auto& child : node->getChildren()) {
            printTree(child, depth + 1, ss);
        }
    }
}

// Generate all valid frontier permutations
std::vector<std::vector<std::string>> PQTree::getFrontiers() const {
    std::vector<std::vector<std::string>> permutations;
    if (root) {
        generatePermutations(root, permutations, {});
    }
    
    // Eliminate duplicates
    std::set<std::vector<std::string>> uniqueSet(permutations.begin(), permutations.end());
    return std::vector<std::vector<std::string>>(uniqueSet.begin(), uniqueSet.end());
}

// Helper to generate permutations for a node
void PQTree::generatePermutations(std::shared_ptr<PQNode> node, 
                                 std::vector<std::vector<std::string>>& permutations, 
                                 std::vector<std::string> current) const {
    if (node->getType() == NodeType::LEAF) {
        current.push_back(node->getLabel());
        permutations.push_back(current);
        return;
    }
    
    if (node->getType() == NodeType::P_NODE) {
        // P-node children can be reordered in any way
        std::vector<std::vector<std::string>> allChildrenLabels;
        for (const auto& child : node->getChildren()) {
            std::vector<std::vector<std::string>> tempPerms;
            generatePermutations(child, tempPerms, {});
            allChildrenLabels.insert(allChildrenLabels.end(), tempPerms.begin(), tempPerms.end());
        }
        
        // Flatten all child labels
        std::vector<std::string> flat;
        for (auto& perm : allChildrenLabels) {
            flat.insert(flat.end(), perm.begin(), perm.end());
        }
        
        // Create all permutations
        std::sort(flat.begin(), flat.end());
        do {
            std::vector<std::string> temp = current;
            temp.insert(temp.end(), flat.begin(), flat.end());
            permutations.push_back(temp);
        } while (std::next_permutation(flat.begin(), flat.end()));
    } else { // Q_NODE
        // Q-node children can only be in order or reverse order
        std::vector<std::vector<std::string>> allChildrenLabels;
        for (const auto& child : node->getChildren()) {
            std::vector<std::vector<std::string>> tempPerms;
            generatePermutations(child, tempPerms, {});
            allChildrenLabels.insert(allChildrenLabels.end(), tempPerms.begin(), tempPerms.end());
        }
        
        // Create forward order
        std::vector<std::string> forward;
        for (auto& perm : allChildrenLabels) {
            forward.insert(forward.end(), perm.begin(), perm.end());
        }
        
        // Create reverse order
        std::vector<std::string> reverse = forward;
        std::reverse(reverse.begin(), reverse.end());
        
        // Add both orders to permutations
        std::vector<std::string> temp1 = current;
        temp1.insert(temp1.end(), forward.begin(), forward.end());
        permutations.push_back(temp1);
        
        if (forward != reverse) {
            std::vector<std::string> temp2 = current;
            temp2.insert(temp2.end(), reverse.begin(), reverse.end());
            permutations.push_back(temp2);
        }
    }
}

// Basic implementation of the reduce operation
bool PQTree::reduce(const std::vector<std::string>& subset) {
    // This is a simplified version - a complete implementation would be more complex
    // For now, just return true to indicate that the constraint is satisfied
    return true;
}

// Simple reordering implementation
void PQTree::reorder() {
    if (!root) return;
    
    // Function to reorder a subtree
    std::function<void(std::shared_ptr<PQNode>)> reorderNode;
    reorderNode = [&](std::shared_ptr<PQNode> node) {
        if (!node) return;
        
        if (node->getType() == NodeType::P_NODE) {
            // For P-nodes, we can reorder children in any way
            auto& children = const_cast<std::vector<std::shared_ptr<PQNode>>&>(node->getChildren());
            // Use modern C++ shuffle instead of deprecated random_shuffle
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(children.begin(), children.end(), g);
        } else if (node->getType() == NodeType::Q_NODE) {
            // For Q-nodes, we can only reverse the order
            auto& children = const_cast<std::vector<std::shared_ptr<PQNode>>&>(node->getChildren());
            if (rand() % 2 == 0) {
                std::reverse(children.begin(), children.end());
            }
        }
        
        // Recursively reorder children
        for (const auto& child : node->getChildren()) {
            reorderNode(child);
        }
    };
    
    reorderNode(root);
}

// Calculate the layout for visualization
void PQTree::computeLayout() {
    if (!root) return;
    
    // Use a simple level-based layout algorithm
    const int LEVEL_HEIGHT = 80;
    const int NODE_WIDTH = 60;
    
    // First, perform a breadth-first traversal to determine levels
    std::map<std::shared_ptr<PQNode>, int> nodeLevels;
    std::queue<std::shared_ptr<PQNode>> queue;
    
    queue.push(root);
    nodeLevels[root] = 0;
    
    while (!queue.empty()) {
        auto node = queue.front();
        queue.pop();
        
        int level = nodeLevels[node];
        
        for (const auto& child : node->getChildren()) {
            nodeLevels[child] = level + 1;
            queue.push(child);
        }
    }
    
    // Next, position nodes based on their level
    std::map<int, std::vector<std::shared_ptr<PQNode>>> levelNodes;
    for (const auto& pair : nodeLevels) {
        levelNodes[pair.second].push_back(pair.first);
    }
    
    for (const auto& pair : levelNodes) {
        int level = pair.first;
        const auto& nodes = pair.second;
        
        // Position nodes evenly at this level
        int totalWidth = nodes.size() * NODE_WIDTH;
        int startX = -totalWidth / 2;
        
        for (size_t i = 0; i < nodes.size(); i++) {
            int x = startX + i * NODE_WIDTH;
            int y = level * LEVEL_HEIGHT;
            nodes[i]->setPosition(x, y);
        }
    }
} 