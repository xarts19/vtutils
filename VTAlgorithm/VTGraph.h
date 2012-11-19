#pragma once

#include <vector>
#include <map>
#include <memory>
#include <iostream>

namespace VT
{
    template <typename TNode, typename TEdge>
    class Graph
    {
    private: // helper classes
        struct Edge
        {
            Edge(const TNode& start, const TNode& end, const TEdge& data) :
                start(start), end(end), data(data)
            { }

            TEdge data;
            TNode start;
            TNode end;
        };

    private: // friends
        template <typename TNode, typename TEdge>
        friend std::ostream& operator<<(std::ostream& os, Graph<TNode, TEdge> graph);

    private: // typedefs
        typedef std::vector<std::shared_ptr<Edge>> IncidenceList;

    private: // data members
        std::map<TNode, IncidenceList> nodes_;

    public: // public interface

        /*
            *FwdIter == TNode
        */
        template <typename FwdIter>
        explicit Graph(FwdIter begin, FwdIter end)
        {
            for (FwdIter it = begin; it != end; ++it)
                nodes_[*it] = IncidenceList();
        }

        bool add_node(const TNode& node)
        {
            if (nodes_.count(node) == 0)
            {
                nodes_[node] = IncidenceList();
                return true;
            }
            else
                return false;
        }

        bool add_edge(const TNode& start, const TNode& end, const TEdge& data)
        {
            for (auto edge_it : nodes_[start])
            {
                if (edge_it->end == end)
                    return false;
            }
            auto edge = std::make_shared<Edge>(start, end, data);
            nodes_[start].push_back(edge);
            nodes_[end].push_back(edge);
            return true;
        }

        TEdge& get_edge_data(const TNode& start, const TNode& end) const
        {
            auto node_pair_it = nodes_.find(start);
            if (node_pair_it == nodes_.end())
                throw std::exception("Node does not exist");

            for (auto& edge_it : node_pair_it->second)
            {
                if (edge_it->end == end)
                    return edge_it->data;
            }

            throw std::exception("Edge does not exist");
        }

        std::vector<TNode> neighbors(const TNode& node) const
        {
            std::vector<TNode> result;
            auto node_pair_it = nodes_.find(node);
            if (node_pair_it == nodes_.end())
                return result;

            // iterate over incidence list
            for (auto& edge_it : node_pair_it->second)
            {
                if (edge_it->start == node)
                    result.push_back(edge_it->end);
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
                if (edge->start == node.first)
                    os << edge->end << " (" << edge->data << ") ";
            }
            os << "]" << std::endl;
        }
        return os;
    }
}
