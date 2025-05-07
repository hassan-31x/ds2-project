#ifndef PQTREE_HPP
#define PQTREE_HPP

#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include "Models.hpp"

enum class NodeType {
    P_NODE,  // Children can be reordered in any way
    Q_NODE,  // Children order can be reversed but not reordered
    LEAF     // Terminal node representing actual items
};

class PQNode {
public:
    PQNode(NodeType type, const std::string& label = "");
    virtual ~PQNode() = default;
    
    NodeType getType() const;
    std::string getLabel() const;
    void setLabel(const std::string& label);
    
    void addChild(std::shared_ptr<PQNode> child);
    const std::vector<std::shared_ptr<PQNode>>& getChildren() const;
    
    // For visualization purposes
    int getX() const;
    int getY() const;
    void setPosition(int x, int y);
    
private:
    NodeType type;
    std::string label;
    std::vector<std::shared_ptr<PQNode>> children;
    
    // For visualization
    int x, y;
};

class PQTree {
public:
    PQTree();
    ~PQTree() = default;
    
    void setRoot(std::shared_ptr<PQNode> node);
    std::shared_ptr<PQNode> getRoot() const;
    
    // Create a leaf node
    std::shared_ptr<PQNode> createLeaf(const std::string& label);
    
    // Create P or Q nodes
    std::shared_ptr<PQNode> createPNode(const std::string& label = "");
    std::shared_ptr<PQNode> createQNode(const std::string& label = "");
    
    // Build a time-ordered tree from sections
    void buildTimeOrderedTree(const std::vector<std::shared_ptr<Section>>& sections);
    
    // Print the tree structure
    std::string print() const;
    
    // Get all valid frontier permutations
    std::vector<std::vector<std::string>> getFrontiers() const;
    
    // Get all permutations with section indices (for scheduling)
    void getAllPermutations(std::vector<std::vector<int>>& permutations);
    
    // PQ Tree operations
    bool reduce(const std::vector<std::string>& subset);
    void reorder();
    
    // For visualization purposes
    void computeLayout();
    
private:
    std::shared_ptr<PQNode> root;
    
    // Helper methods
    void printTree(std::shared_ptr<PQNode> node, int depth, std::stringstream& ss) const;
    void generatePermutations(std::shared_ptr<PQNode> node, 
                             std::vector<std::vector<std::string>>& permutations, 
                             std::vector<std::string> current) const;
};

#endif // PQTREE_HPP 