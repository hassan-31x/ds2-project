#include "PQTree.hpp"
#include <algorithm>
#include <queue>
#include <map>
#include <random>

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

// Basic implementation of the reduce operation
// This is a simplified version - a complete implementation would be more complex
bool PQTree::reduce(const std::vector<std::string>& subset) {
    // Silence unused parameter warning
    (void)subset;
    
    // For now, just return true to indicate that the constraint is satisfied
    // A real implementation would update the tree structure
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