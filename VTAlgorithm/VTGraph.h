#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <exception>

// TODO:
//      * use custom allocator to fight memory fragmentation and slow destruction times
//        when using huge graphs (map allocates each node separately by default)

namespace VT
{
    template <typename TNode, typename TEdge>
    class Graph
    {
    private: // helper classes
        struct Edge
        {
            Edge(const TNode& end, const TEdge& data) :
                end(end), data(data)
            { }

            TNode end;
            TEdge data;
        };

    private: // typedefs
        typedef std::vector<Edge> IncidenceList;

    private: // data members
        std::map<TNode, IncidenceList> nodes_;
        unsigned int prealloc_edges_;

    private: // helper methods
        const IncidenceList& get_inc_list(const TNode& node) const
        {
            auto node_pair_it = nodes_.find(node);
            if (node_pair_it == nodes_.end())
                throw std::runtime_error("Node does not exist");
            return node_pair_it->second;
        }

    private: // friends
        template <typename TNode, typename TEdge>
        friend std::ostream& operator<<(std::ostream& os, Graph<TNode, TEdge> graph);

    public: // public interface

        Graph() : prealloc_edges_(0) { }

        /*
            *FwdIter ~ TNode
        */
        template <typename FwdIter>
        explicit Graph(FwdIter begin, FwdIter end) : prealloc_edges_(0)
        {
            for (FwdIter it = begin; it != end; ++it)
                nodes_[*it] = IncidenceList();
        }

        // set expected number of outgoing edges for each node so that
        // they can be preallocated
        void set_prealloc_edges(unsigned int min_edge_count)
        {
            prealloc_edges_ = min_edge_count;
        }

        bool add_node(const TNode& node)
        {
            if (nodes_.count(node) == 0)
            {
                nodes_[node] = IncidenceList();
                if (prealloc_edges_)
                    nodes_[node].reserve(prealloc_edges_);
                return true;
            }
            else
                return false;
        }

        /*
        // need to traverse whole graph to find incoming edges
        // not optimal so not implemented
        bool remove_node(const TNode& node)
        {
            return nodes_.erase(node) > 0;
        }
        */

        bool add_edge(const TNode& start, const TNode& end, const TEdge& data)
        {
            for (const Edge& edge : nodes_[start])
            {
                if (edge.end == end)
                    return false;       // edge already exists
            }
            nodes_[start].emplace_back(end, data);
            return true;
        }

        const TEdge& get_edge_data(const TNode& start, const TNode& end) const
        {
            const IncidenceList& edges = get_inc_list(start);

            for (const Edge& edge : edges)
            {
                if (edge.end == end)
                    return edge.data;
            }

            throw std::runtime_error("Edge does not exist");
        }

        /*
            Nodes reachable from this node.
        */
        std::vector<TNode> neighbors(const TNode& node) const
        {
            std::vector<TNode> result;

            const IncidenceList& edges = get_inc_list(node);

            // iterate over incidence list
            for (const Edge& edge : edges)
            {
                result.push_back(edge.end);
            }
            return result;
        }
    };

    template <typename TNode, typename TEdge>
    std::ostream& operator<<(std::ostream& os, Graph<TNode, TEdge> graph)
    {
        for (auto& node : graph.nodes_)
        {
            os << node.first << ": [ ";
            for (auto& edge : node.second)
            {
                os << edge.end << " (" << edge.data << ") ";
            }
            os << "]" << std::endl;
        }
        return os;
    }
}
