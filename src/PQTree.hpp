#ifndef PQTREE_HPP
#define PQTREE_HPP

#include <vector>
#include <memory>
#include <string>

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
    
    // PQ Tree operations
    bool reduce(const std::vector<std::string>& subset);
    void reorder();
    
    // For visualization purposes
    void computeLayout();
    
private:
    std::shared_ptr<PQNode> root;
};

#endif // PQTREE_HPP 