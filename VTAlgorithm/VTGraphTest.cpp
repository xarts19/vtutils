#include "VTGraph.h"

#include <iostream>

using namespace std;

inline void vt_graph_test()
{
    vector<int> v;
    for (int i = 0; i < 10; ++i)
        v.push_back(i);
    VT::Graph<int, int> g(v.cbegin(), v.cend());
    g.add_node(10);
    g.add_edge(1, 5, 100);
    g.add_edge(5, 10, 10);
    g.add_edge(1, 6, 60);
    g.add_edge(6, 10, 60);

    cout << g;
}