#pragma once

#include "VTGraph.h"

#include <deque>
#include <set>
#include <memory>
#include <stack>
#include <queue>

namespace VT
{
    namespace detail_
    {
        template <typename TNode>
        struct PathElem
        {
            PathElem(const TNode& node_, std::shared_ptr<PathElem> prev_) :
				node(node_),
				prev(prev_)
			{ }

            TNode node;
            std::shared_ptr<PathElem> prev;
        };


        template <typename Elem>
        struct QueueAdapter : public std::queue<Elem>
        {
            Elem& top() { return this->front(); }
            const Elem& top() const { return this->front(); }
        };


        template <typename TNode, typename TEdge, typename TPred, typename TQueue>
        std::deque<TNode> generalized_search_helper(const Graph<TNode, TEdge>& graph,
                                                    const TNode& start,
                                                    TPred is_dest,
                                                    TQueue queue)
        {
            std::set<TNode> visited;
            queue.push(std::make_shared<PathElem<TNode>>(start, std::shared_ptr<PathElem<TNode>>()));
            visited.insert(start);

            std::shared_ptr<PathElem<TNode>> current;

            for (;;)
            {
                if (queue.empty())
                    return std::deque<TNode>(); // path not found

                current = queue.top();
                queue.pop();

                if (is_dest(current->node))
                    break;

                for (auto& node : graph.neighbors(current->node))
                {
                    if (visited.count(node) > 0)
                        continue;
                    visited.insert(node);
                    queue.push( std::make_shared<PathElem<TNode>>(node, current) );
                }
            }

            std::deque<TNode> result;
            // extract resulting path
            while (current)
            {
                result.push_front(current->node);
                current = current->prev;
            }

            return result;
        }
    }

    /*
        TPred:   bool is_destination(TNode);

        if result is empty container - path was not found;
    */
    template <typename TNode, typename TEdge, typename TPred>
    std::deque<TNode> breadth_first_search(const Graph<TNode, TEdge>& graph, const TNode& start, TPred is_dest)
    {
        return generalized_search_helper(graph, start, is_dest, detail_::QueueAdapter<std::shared_ptr<detail_::PathElem<TNode>>>());
    }
    template <typename TNode, typename TEdge, typename TPred>
    std::deque<TNode> depth_first_search(const Graph<TNode, TEdge>& graph, const TNode& start, TPred is_dest)
    {
        return generalized_search_helper(graph, start, is_dest, std::stack<std::shared_ptr<detail_::PathElem<TNode>>>());
    }
    
    /*
        TPred:      bool is_destination(TNode);
        TDistFunc:  int (TNode start, TNode end) - distance between start and end
        THeurFunc:  int (TNode node) - approximate ditance to goal from this node
                    (heuristic, should always be smaller than real distance to be admissable so that
                    search will always find optimal solution)
    */
    template <template <typename, typename> class Graph, typename TNode, typename TEdge,
                                    typename TPred, typename TDistFunc, typename THeurFunc>
    std::deque<TNode> a_star_search(const Graph<TNode, TEdge>& graph,
                                    const TNode& start,
                                    TPred is_dest,
                                    TDistFunc dist_func,
                                    THeurFunc heur_func)
    {
        std::set<TNode> closedset;      // the set of nodes already evaluated
        std::set<TNode> openset;        // the set of nodes to be evaluated
        
        std::map<TNode, double> g_score;   // cost from some node along best known path to goal
        std::map<TNode, double> f_score;   // estimated total cost from some start to goal through some node.

        std::map<TNode, TNode> came_from;   // the map of navigated nodes

        auto calc_f_score = [&g_score, &heur_func] (TNode node) { return g_score[node] + heur_func(node); };

        g_score[start] = 0;
        f_score[start] = calc_f_score(start);
        openset.insert(start);

        TNode current;

        for (;;)
        {
            if (openset.empty())
                return std::deque<TNode>(); // path not found

            // set current to the node in openset having the lowest f_score[] value
            double min = std::numeric_limits<int>::max();
            for (const TNode& node: openset)
            {
                if (f_score[node] < min)
                    current = node;
            }
            
            openset.erase(current);
            closedset.insert(current);

            if (is_dest(current))
                break;

            for (auto& node : graph.neighbors(current))
            {
                if (closedset.count(node) > 0)
                    continue;   // already was here

                double tentative_g_score = g_score[current] + dist_func(current, node);

                if (openset.count(node) == 0 || tentative_g_score < g_score[node])
                {
                    came_from[node] = current;
                    g_score[node] = tentative_g_score;
                    f_score[node] = calc_f_score(node);
                    if (openset.count(node) == 0)
                         openset.insert(node);
                }
            }
        }

        // extract resulting path
        std::deque<TNode> result;
        for (;;)
        {
            result.push_front(current);
            if (current == start)
                break;
            current = came_from[current];
        }
        return result;
    }
}
