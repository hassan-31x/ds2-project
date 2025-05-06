#include "pq_tree.h"
#include <iostream>
#include <queue>

// PQNode implementation
PQNode::PQNode(NodeType t, const std::string &lbl) : type(t), label(lbl) {}

PQNode::~PQNode()
{
    clearChildren();
}

void PQNode::addChild(std::shared_ptr<PQNode> child)
{
    children.push_back(child);
    child->setParent(shared_from_this());
}

void PQNode::removeChild(std::shared_ptr<PQNode> child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
    {
        children.erase(it);
    }
}

void PQNode::replaceChild(std::shared_ptr<PQNode> oldChild, std::shared_ptr<PQNode> newChild)
{
    auto it = std::find(children.begin(), children.end(), oldChild);
    if (it != children.end())
    {
        *it = newChild;
        newChild->setParent(shared_from_this());
    }
}

std::vector<std::string> PQNode::getFrontier()
{
    std::vector<std::string> frontier;

    if (isLeaf())
    {
        frontier.push_back(label);
    }
    else
    {
        for (auto &child : children)
        {
            auto childFrontier = child->getFrontier();
            frontier.insert(frontier.end(), childFrontier.begin(), childFrontier.end());
        }
    }

    return frontier;
}

// PQTree implementation
PQTree::PQTree() : root(std::make_shared<PQNode>(NodeType::P_NODE)) {}

PQTree::~PQTree()
{
    root.reset();
    leaves.clear();
}

void PQTree::createFromUniversalSet(const std::vector<std::string> &elements)
{
    root = std::make_shared<PQNode>(NodeType::P_NODE);
    leaves.clear();

    // Create leaf nodes for each element
    for (const auto &elem : elements)
    {
        auto leaf = std::make_shared<PQNode>(NodeType::LEAF, elem);
        root->addChild(leaf);
        leaves[elem] = leaf;
    }
}

bool PQTree::reduce(const std::set<std::string> &subset)
{
    // This is a simplified PQ tree reduction algorithm
    // A full implementation would involve multiple patterns and templates

    // Mark all nodes in the subset
    std::set<std::shared_ptr<PQNode>> markedLeaves;
    for (const auto &elem : subset)
    {
        auto it = leaves.find(elem);
        if (it != leaves.end())
        {
            markedLeaves.insert(it->second);
        }
    }

    if (markedLeaves.size() != subset.size())
    {
        return false; // Some elements in the subset are not in the tree
    }

    // Simplified reduction for demo purposes
    // In a real implementation, this would involve applying templates
    // and bubble-up operations

    // For now, we just check if all elements are consecutive in the current arrangement
    auto frontier = getFrontier();

    // Find the first and last occurrence of a marked element
    int first = -1, last = -1;
    for (int i = 0; i < frontier.size(); i++)
    {
        if (subset.find(frontier[i]) != subset.end())
        {
            if (first == -1)
                first = i;
            last = i;
        }
    }

    // Check if all elements between first and last are in the subset
    for (int i = first; i <= last; i++)
    {
        if (subset.find(frontier[i]) == subset.end())
        {
            return false;
        }
    }

    return true;
}

std::vector<std::vector<std::string>> PQTree::getPossibleArrangements()
{
    // A full implementation would enumerate all possible arrangements
    // based on the PQ tree structure

    // For now, we just return the current frontier as the only arrangement
    std::vector<std::vector<std::string>> arrangements;
    arrangements.push_back(getFrontier());
    return arrangements;
}

std::vector<std::string> PQTree::getFrontier()
{
    return root->getFrontier();
}

void PQTree::print()
{
    std::cout << "PQ Tree Frontier: ";
    auto frontier = getFrontier();
    for (const auto &elem : frontier)
    {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}