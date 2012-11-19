#include "VTSearch.h"

#include "../std_pretty_printer.h"

#include <iostream>
#include <vector>

using namespace std;

inline void vt_bfs_test()
{
    vector<int> v;
    for (int i = 0; i < 10; ++i)
        v.push_back(i);
    VT::Graph<int, int> g(v.cbegin(), v.cend());
    g.add_node(10);
    g.add_edge(1, 5, 4);
    g.add_edge(5, 10, 5);
    g.add_edge(6, 10, 4);
    g.add_edge(5, 6, 1);
    g.add_edge(6, 7, 1);
    g.add_edge(7, 9, 2);
    g.add_edge(9, 10, 1);
    g.add_edge(5, 1, 4);

    cout << g;

    auto result = VT::breadth_first_search(g, 1, [](int node) { return node == 10; });
    print_line(cout, result);

    result = VT::depth_first_search(g, 1, [](int node) { return node == 10; });
    print_line(cout, result);

    result = VT::a_star_search(g, 1, [](int node) { return node == 10; },
                                     [](int, int, int edge_data) { return edge_data; },
                                     [](int node) { return 10 - node; } );
    print_line(cout, result);
}

int main()
{
    vt_bfs_test();
    getchar();
    return 0;
}