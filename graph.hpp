#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stack>
#include <stddef.h>
#include <unordered_set>
#include <utility>
#include <vector>

template<typename ElementType>
struct Span
{
    using iterator = ElementType*;

    template<typename Container>
    Span(Container& c) : begin_(c.data()), end_(begin_ + c.size())
    {
    }

    iterator begin() const { return begin_; }
    iterator end() const { return end_; }

private:
    iterator begin_;
    iterator end_;
};

template<typename ElementType>
class IdMap
{
public:
    using iterator = typename std::vector<ElementType>::iterator;
    using const_iterator = typename std::vector<ElementType>::const_iterator;

    // Iterators

    const_iterator begin() const { return elements_.begin(); }
    const_iterator end() const { return elements_.end(); }

    // Element access

    Span<const ElementType> elements() const { return elements_; }

    // Capacity

    bool   empty() const { return sorted_ids_.empty(); }
    size_t size() const { return sorted_ids_.size(); }

    // Modifiers

    std::pair<iterator, bool> insert(int id, const ElementType& element);
    std::pair<iterator, bool> insert(int id, ElementType&& element);
    size_t                    erase(int id);
    void                      clear();

    // Lookup

    iterator       find(int id);
    const_iterator find(int id) const;
    bool           contains(int id) const;

private:
    std::vector<ElementType> elements_;
    std::vector<int>         sorted_ids_;
};

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int          id,
    const ElementType& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(
            std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, element), true);
}

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int     id,
    ElementType&& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(
            std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, std::move(element)), true);
}

template<typename ElementType>
size_t IdMap<ElementType>::erase(const int id)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound == sorted_ids_.end() || id != *lower_bound)
    {
        return 0ull;
    }

    auto erase_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.erase(lower_bound);
    elements_.erase(erase_element_at);

    return 1ull;
}

template<typename ElementType>
void IdMap<ElementType>::clear()
{
    elements_.clear();
    sorted_ids_.clear();
}

template<typename ElementType>
typename IdMap<ElementType>::iterator IdMap<ElementType>::find(const int id)
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
               ? elements_.end()
               : std::next(elements_.begin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
typename IdMap<ElementType>::const_iterator IdMap<ElementType>::find(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
               ? elements_.cend()
               : std::next(elements_.cbegin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
bool IdMap<ElementType>::contains(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);

    if (lower_bound == sorted_ids_.cend())
    {
        return false;
    }

    return *lower_bound == id;
}

// a very simple directional graph
template<typename NodeType>
class Graph
{
public:
    Graph() : current_id_(0), nodes_(), edges_from_node_(), node_neighbors_(), edges_() {}

    struct Edge
    {
        int id;
        int from, to;

        Edge() = default;
        Edge(const int id, const int f, const int t) : id(id), from(f), to(t) {}

        inline int  opposite(const int n) const { return n == from ? to : from; }
        inline bool contains(const int n) const { return n == from || n == to; }
    };

    // Element access

    NodeType&        node(int node_id);
    const NodeType&  node(int node_id) const;
    Span<const int>  neighbors(int node_id) const;
    Span<const Edge> edges() const;

    // Capacity

    size_t num_edges_from_node(int node_id) const;

    // Modifiers

    int  insert_node(const NodeType& node);
    void erase_node(int node_id);

    int  insert_edge(int from, int to);
    void erase_edge(int edge_id);

    bool node_exists(const int id) const;
    bool edge_exists(int from, int to) const;
    void remove_disconnected_edges(int start_node);

private:
    int current_id_;
    // These contains map to the node id
    IdMap<NodeType>         nodes_;
    IdMap<int>              edges_from_node_;
    IdMap<std::vector<int>> node_neighbors_;

    // This container maps to the edge id
    IdMap<Edge> edges_;
};

template<typename NodeType>
NodeType& Graph<NodeType>::node(const int id)
{
    return const_cast<NodeType&>(static_cast<const Graph*>(this)->node(id));
}

template<typename NodeType>
const NodeType& Graph<NodeType>::node(const int id) const
{
    const auto iter = nodes_.find(id);
    //assert(iter != nodes_.end());
    return *iter;
}

template<typename NodeType>
Span<const int> Graph<NodeType>::neighbors(int node_id) const {
    auto iter = node_neighbors_.find(node_id);
    if (iter == node_neighbors_.end()) {
        static const std::vector<int> empty_vector; // Define an empty vector to represent no neighbors
        return Span<const int>(empty_vector);      // Use the empty vector to create an empty span
    }
    return *iter;
}


template<typename NodeType>
Span<const typename Graph<NodeType>::Edge> Graph<NodeType>::edges() const
{
    return edges_.elements();
}

template<typename NodeType>
size_t Graph<NodeType>::num_edges_from_node(const int id) const
{
    auto iter = edges_from_node_.find(id);
    assert(iter != edges_from_node_.end());
    return *iter;
}

template<typename NodeType>
bool Graph<NodeType>::node_exists(const int id) const
{
    return nodes_.contains(id);
}

template<typename NodeType>
int Graph<NodeType>::insert_node(const NodeType& node)
{
    const int id = current_id_++;
    assert(!nodes_.contains(id));
    nodes_.insert(id, node);
    edges_from_node_.insert(id, 0);
    node_neighbors_.insert(id, std::vector<int>());
    std::cerr << "insert node id: " << id << std::endl;
    return id;
}

template<typename NodeType>
void Graph<NodeType>::erase_node(const int id)
{

    // first, remove any potential dangling edges
    {
        static std::vector<int> edges_to_erase;

        for (const Edge& edge : edges_.elements())
        {
            if (edge.contains(id))
            {
                edges_to_erase.push_back(edge.id);
            }
        }

        for (const int edge_id : edges_to_erase)
        {
            erase_edge(edge_id);
        }

        edges_to_erase.clear();
    }

    nodes_.erase(id);
    edges_from_node_.erase(id);
    node_neighbors_.erase(id);
}

template<typename NodeType>
int Graph<NodeType>::insert_edge(const int from, const int to)
{
    if (from == to) {
        std::cerr << "Error: Self-loop detected (from = " << from << ", to = " << to << ")" << std::endl;
        return -1;
    }

    const int id = current_id_++;
    assert(!edges_.contains(id));
    assert(nodes_.contains(from));
    //assert(nodes_.contains(to));
    edges_.insert(id, Edge(id, from, to));

    // update neighbor count
    assert(edges_from_node_.contains(from));
    *edges_from_node_.find(from) += 1;
    // update neighbor list
    assert(node_neighbors_.contains(from));
    node_neighbors_.find(from)->push_back(to);

    return id;
}

template<typename NodeType>
void Graph<NodeType>::erase_edge(const int edge_id)
{
    if (!edges_.contains(edge_id))
    {
        std::cerr << "Edge with ID " << edge_id << " does not exist.\n";
        return;
    }

    const Edge& edge = *edges_.find(edge_id);

    // update neighbor count
    assert(edges_from_node_.contains(edge.from));
    int& edge_count = *edges_from_node_.find(edge.from);
    assert(edge_count > 0);
    edge_count -= 1;

    // update neighbor list
    {
        assert(node_neighbors_.contains(edge.from));
        auto neighbors = node_neighbors_.find(edge.from);
        auto iter = std::find(neighbors->begin(), neighbors->end(), edge.to);
        assert(iter != neighbors->end());
        neighbors->erase(iter);
    }

    edges_.erase(edge_id);
}

template<typename NodeType>
bool Graph<NodeType>::edge_exists(int from, int to) const {
    for (const auto& edge : edges_) {
        if (edge.from == from && edge.to == to) {
            return true;
        }
    }
    return false;
}

template<typename NodeType>
void Graph<NodeType>::remove_disconnected_edges(int start_node) {
    // Step 1: Find all reachable nodes using DFS
    // Initialize a set to keep track of all nodes that can be visited from the start node
    std::unordered_set<int> reachable_nodes;
    dfs_traverse(*this, start_node, [&reachable_nodes](int node) {
        reachable_nodes.insert(node);
    });

    // Step 2: Identify valid edges connecting reachable nodes
    // Store IDs of edges that connect only reachable nodes
    std::vector<int> edges_to_keep;
    for (const auto& edge : edges_.elements()) {
        if (reachable_nodes.count(edge.from) && reachable_nodes.count(edge.to)) {
            edges_to_keep.push_back(edge.id);
        }
    }

    // Step 3: Remove edges that are not in the list of valid edges
    // Any edge not in the `edges_to_keep` list is considered disconnected and will be removed
    std::vector<int> edges_to_erase;
    for (const auto& edge : edges_.elements()) {
        if (std::find(edges_to_keep.begin(), edges_to_keep.end(), edge.id) == edges_to_keep.end()) {
            edges_to_erase.push_back(edge.id);
        }
    }

    // Delete each disconnected edge from the graph
    for (int edge_id : edges_to_erase) {
        erase_edge(edge_id);
    }
}

template<typename NodeType, typename Visitor>
void dfs_traverse(const Graph<NodeType>& graph, const int start_node, Visitor visitor)
{
    std::stack<int> stack;

    stack.push(start_node);

    while (!stack.empty())
    {
        const int current_node = stack.top();
        stack.pop();

        visitor(current_node);

        for (const int neighbor : graph.neighbors(current_node))
        {
            stack.push(neighbor);
        }
    }
}
