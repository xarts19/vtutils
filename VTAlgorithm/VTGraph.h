#pragma once

#include "../VTAllocator.h"
#include "../VTSortedVectorMap.h"

#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <memory>

// TODO:
//      * use custom allocator to fight memory fragmentation and slow destruction times
//        when using huge graphs (map allocates each node separately by default)
//      * provide a specialization that uses sorted vector instead of map

namespace VT
{
    template <typename TNode, typename TEdge>
    struct GraphTraitsBase
    {
        struct Edge
        {
            Edge(const TNode& end_, const TEdge& data_) :
                end(end_), data(data_)
            { }

            TNode end;
            TEdge data;
        };

        typedef std::vector<Edge>                       IncidenceListType;
        typedef std::unique_ptr<IncidenceListType>      IncidenceListPtrType;
        typedef std::pair<TNode, IncidenceListPtrType>  NodeMapElemType;
        typedef std::allocator<NodeMapElemType>         NodeMapAllocatorType;
    };


    template<typename TNode, typename TEdge>
    struct GraphTraitsMap : public GraphTraitsBase<TNode, TEdge>
    {
        using typename GraphTraitsBase<TNode, TEdge>::Edge;
        using typename GraphTraitsBase<TNode, TEdge>::IncidenceListType;
        using typename GraphTraitsBase<TNode, TEdge>::IncidenceListPtrType;
        using typename GraphTraitsBase<TNode, TEdge>::NodeMapElemType;
        //using typename GraphTraitsBase<TNode, TEdge>::NodeMapAllocatorType;

        typedef VT::SSAllocator<NodeMapElemType> NodeMapAllocatorType;

        typedef std::map<TNode,
                         IncidenceListPtrType, 
                         std::less<TNode>,
                         NodeMapAllocatorType>   NodeMapType;

        static void reserve(NodeMapType&, typename NodeMapType::size_type)
        {
            // do nothing
        }

        static void sort(NodeMapType&)
        {
            // do nothing
        }
    };


    template<typename TNode, typename TEdge>
    struct GraphTraitsVector : public GraphTraitsBase<TNode, TEdge>
    {
        using typename GraphTraitsBase<TNode, TEdge>::Edge;
        using typename GraphTraitsBase<TNode, TEdge>::IncidenceListType;
        using typename GraphTraitsBase<TNode, TEdge>::IncidenceListPtrType;
        using typename GraphTraitsBase<TNode, TEdge>::NodeMapElemType;
        using typename GraphTraitsBase<TNode, TEdge>::NodeMapAllocatorType;

        typedef VT::SortedVectorMap<TNode,
                                    IncidenceListPtrType, 
                                    NodeMapAllocatorType>   NodeMapType;

        static void reserve(NodeMapType& map, typename NodeMapType::size_type n)
        {
            map.reserve(n);
        }

        static void sort(NodeMapType& map)
        {
            map.sort();
        }
    };

    template <typename TNode, typename TEdge, typename GraphTraits>
    class GraphBase
    {
    private:
    // typedefs
        typedef typename GraphTraits::Edge                  Edge;
        typedef typename GraphTraits::IncidenceListType     IncidenceListType;
        typedef typename GraphTraits::IncidenceListPtrType  IncidenceListPtrType;
        typedef typename GraphTraits::NodeMapElemType       NodeMapElemType;
        typedef typename GraphTraits::NodeMapAllocatorType  NodeMapAllocatorType;
        typedef typename GraphTraits::NodeMapType           NodeMapType;
    
    // data members
        NodeMapType nodes_;

    // helper methods
        const IncidenceListType& get_inc_list(const TNode& node) const
        {
            typename NodeMapType::const_iterator map_it = nodes_.find(node);
            if (map_it == nodes_.end())
                throw std::runtime_error("Node does not exist");
            assert(map_it->second);
            return *(map_it->second);
        }

        IncidenceListType& get_inc_list(const TNode& node)
        {
            typename NodeMapType::iterator map_it = nodes_.find(node);
            if (map_it == nodes_.end())
                throw std::runtime_error("Node does not exist");
            assert(map_it->second);
            return *(map_it->second);
        }

        bool empty_node(const TNode& node)
        {
            auto it = nodes_.emplace(node, IncidenceListPtrType(new IncidenceListType()));
            return it.second;
        }

    // not implemented
        GraphBase(const GraphBase& other);
        GraphBase& operator=(const GraphBase& rhs);

    public: // public interface
        GraphBase() { }

        /*
            *FwdIter ~ TNode
        */
        template <typename FwdIter>
        explicit GraphBase(FwdIter begin, FwdIter end)
        {
            for (FwdIter it = begin; it != end; ++it)
                empty_node(*it);
        }

        void reserve(typename NodeMapType::size_type n)
        {
            GraphTraits::reserve(nodes_, n);
        }

        void sort()
        {
            GraphTraits::sort(nodes_);
        }

        bool add_node(const TNode& node)
        {
            return empty_node(node);
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

        void print(std::ostream& os) const
        {
            for (auto& node : nodes_)
            {
                os << node.first << ": [ ";
                assert(node.second);
                for (auto& edge : *node.second)
                {
                    os << edge.end << " (" << edge.data << ") ";
                }
                os << "]" << std::endl;
            }
        }
    };


    template <typename TNode, typename TEdge>
    class Graph : public GraphBase<TNode, TEdge, GraphTraitsMap<TNode, TEdge>>
    {
    public:
        Graph() : GraphBase<TNode, TEdge, GraphTraitsMap<TNode, TEdge>>() { }
        /*
            *FwdIter ~ TNode
        */
        template <typename FwdIter>
        explicit Graph(FwdIter begin, FwdIter end) : 
			GraphBase<TNode, TEdge, GraphTraitsMap<TNode, TEdge>>(begin, end) { }

    private:
    // not implemented
        Graph(const Graph& other);
        Graph& operator=(const Graph& rhs);
    };


    template <typename TNode, typename TEdge>
    class VectorGraph : public GraphBase<TNode, TEdge, GraphTraitsVector<TNode, TEdge>>
    {
    public:
        VectorGraph() : GraphBase<TNode, TEdge, GraphTraitsVector<TNode, TEdge>>() { }
        /*
            *FwdIter ~ TNode
        */
        template <typename FwdIter>
        explicit VectorGraph(FwdIter begin, FwdIter end) :
			GraphBase<TNode, TEdge, GraphTraitsVector<TNode, TEdge>>(begin, end) { }

    private:
    // not implemented
        VectorGraph(const VectorGraph& other);
        VectorGraph& operator=(const VectorGraph& rhs);
    };


    template <template <typename, typename> class Graph, typename TNode, typename TEdge>
    std::ostream& operator<<(std::ostream& os, const Graph<TNode, TEdge>& graph)
    {
        graph.print(os);
        return os;
    }
}
