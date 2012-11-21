#include "VTSearch.h"

#include "../std_pretty_printer.h"

#include <iostream>
#include <vector>

using namespace std;

struct Node
{
    int x;
    int y;
    Node() { }
    Node(int x, int y) : x(x), y(y) { }
    bool operator==(const Node& other) const { return (x == other.x && y == other.y); }
    bool operator<(const Node& other) const { return (x < other.x || x == other.x && y < other.y); }
    friend ostream& operator<<(ostream& os, const Node& n)
    {
        os << "(" << n.x << ", " << n.y << ")";
        return os;
    }
};

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
    g.add_edge(1, 1, 0);

    cout << g;

    auto result = VT::breadth_first_search(g, 1, [](int node) { return node == 10; });
    print_line(cout, result);

    result = VT::depth_first_search(g, 1, [](int node) { return node == 10; });
    print_line(cout, result);

    cout << "A*" << endl;

    auto dist = [](const Node& a, const Node& b) { double dx = a.x - b.x; double dy = a.y - b.y; return sqrt(dx*dx+dy*dy); };

    //
    //    *-*-*
    //    |  /|
    //    *-*-*
    //
    Node n01 = Node(0, 1); Node n11 = Node(1, 1); Node n21 = Node(2, 1);
    Node n00 = Node(0, 0); Node n10 = Node(1, 0); Node n20 = Node(2, 0);
    VT::Graph<Node, double> g1;
    g1.add_node(n00); g1.add_node(n01); g1.add_node(n10);
    g1.add_node(n11); g1.add_node(n20); g1.add_node(n21);
    
    g1.add_edge(n00, n01, dist(n00, n01));
    g1.add_edge(n01, n11, dist(n01, n11));
    g1.add_edge(n11, n21, dist(n11, n21));
    g1.add_edge(n00, n10, dist(n00, n10));
    g1.add_edge(n10, n20, dist(n10, n20));
    g1.add_edge(n20, n21, dist(n20, n21));
    g1.add_edge(n10, n21, dist(n10, n21));

    cout << g1;

    std::deque<Node> r;
    r = VT::a_star_search(g1, n00, [&n21](const Node& node) { return node == n21; },
                                   [&g1](const Node& a, const Node& b) { return g1.get_edge_data(a, b); },
                                   [&dist, &n21](const Node& node) { return dist(node, n21); }
                         );
    print_line(cout, r);



    VT::Graph<Node, double> g2;
    int size = 100;
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            g2.add_node(Node(i, j));
            if (i > 0)
            {
                g2.add_edge(Node(i, j), Node(i-1, j), dist(Node(i, j), Node(i-1, j)));
                g2.add_edge(Node(i-1, j), Node(i, j), dist(Node(i-1, j), Node(i, j)));
            }
            if (j > 0)
            {
                g2.add_edge(Node(i, j), Node(i, j-1), dist(Node(i, j), Node(i, j-1)));
                g2.add_edge(Node(i, j-1), Node(i, j), dist(Node(i, j-1), Node(i, j)));
            }
        }
    }

    r = VT::a_star_search(g2, Node(0, 0), [&](const Node& node) { return node == Node(size-1, size-1); },
                                          [&](const Node& a, const Node& b) { return g2.get_edge_data(a, b); },
                                          [&](const Node& node) { return dist(node, Node(size-1, size-1)); }
                         );
    print_line(cout, r);
    
    std::map<int, std::unique_ptr<int>> a;
}

int main()
{
    vt_bfs_test();
    getchar();
    return 0;
}