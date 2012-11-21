#pragma once

#include "../VTAllocator.h"
#include "../VTSortedVectorMap.h"

#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <memory>

// TODO:
//      * use custom allocator to fight memory fragmentation and slow destruction times
//        when using huge graphs (map allocates each node separately by default)
//      * provide a specialization that uses sorted vector instead of map

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
        typedef std::vector<Edge>                       IncidenceListType;
        typedef std::unique_ptr<IncidenceListType>      IncidenceListPtrType;
        typedef std::pair<TNode, IncidenceListPtrType>  NodeMapElemType;
        typedef VT::SSAllocator<NodeMapElemType>        NodeMapAllocatorType;
        typedef std::map<TNode,
                         IncidenceListPtrType, 
                         std::less<TNode>,
                         NodeMapAllocatorType>          NodeMapType;

    private: // data members
        NodeMapType nodes_;

    private: // helper methods
        const IncidenceListType& get_inc_list(const TNode& node) const
        {
            NodeMapType::const_iterator map_it = nodes_.find(node);
            if (map_it == nodes_.end())
                throw std::runtime_error("Node does not exist");
            assert(map_it->second);
            return *(map_it->second);
        }

        IncidenceListType& get_inc_list(const TNode& node)
        {
            NodeMapType::iterator map_it = nodes_.find(node);
            if (map_it == nodes_.end())
                throw std::runtime_error("Node does not exist");
            assert(map_it->second);
            return *(map_it->second);
        }

        void empty_node(const TNode& node)
        {
            nodes_.emplace(node, IncidenceListPtrType(new IncidenceListType()));
        }

        template <typename MapType>
        struct reserve_helper
        {
            void reserve(const MapType&, typename MapType::size_type)
            {
                // do nothing
            }
        };

        template <template<typename, typename, typename> class SortedVectorMap, 
            typename K, typename V>
        struct reserve_helper<SortedVectorMap<K, V, std::allocator<std::pair<K, V>>>>
        {
            void reserve(const SortedVectorMap& map, typename SortedVectorMap::size_type n)
            {
                map.reserve(n);
            }
        };

    private: // not implemented
        Graph(const Graph& other);
        Graph& operator=(const Graph& rhs);

    private: // friends
        template <typename TNode, typename TEdge>
        friend std::ostream& operator<<(std::ostream& os, const Graph<TNode, TEdge>& graph);

    public: // public interface

        Graph() { }

        /*
            *FwdIter ~ TNode
        */
        template <typename FwdIter>
        explicit Graph(FwdIter begin, FwdIter end)
        {
            for (FwdIter it = begin; it != end; ++it)
                empty_node(*it);
        }

        void reserve(typename NodeMapType::size_type n)
        {
            reserve_helper<NodeMapType>::reserve(nodes_, n);
        }

        bool add_node(const TNode& node)
        {
            if (nodes_.count(node) == 0)
            {
                empty_node(node);
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
            IncidenceListType& inc_list = get_inc_list(start);
            for (const Edge& edge : inc_list)
            {
                if (edge.end == end)
                    return false;       // edge already exists
            }
            inc_list.emplace_back(end, data);
            return true;
        }

        const TEdge& get_edge_data(const TNode& start, const TNode& end) const
        {
            const IncidenceListType& edges = get_inc_list(start);

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

            const IncidenceListType& edges = get_inc_list(node);

            // iterate over incidence list
            for (const Edge& edge : edges)
            {
                result.push_back(edge.end);
            }
            return result;
        }
    };

    template <typename TNode, typename TEdge>
    std::ostream& operator<<(std::ostream& os, const Graph<TNode, TEdge>& graph)
    {
        for (auto& node : graph.nodes_)
        {
            os << node.first << ": [ ";
            assert(node.second);
            for (auto& edge : *node.second)
            {
                os << edge.end << " (" << edge.data << ") ";
            }
            os << "]" << std::endl;
        }
        return os;
    }
}
