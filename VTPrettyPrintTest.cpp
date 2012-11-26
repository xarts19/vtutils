#include "VTPrettyPrint.h"

using namespace std;

struct special_formatter {
    template <typename T> void    prefix(std::ostream& os, const T& t) const {
        default_formatter().prefix(os, t);
    }
    template <typename T> void separator(std::ostream& os, const T& t) const {
        default_formatter().separator(os, t);
    }
    template <typename T> void    suffix(std::ostream& os, const T& t) const {
        default_formatter().suffix(os, t);
    }
    template <typename T> void   element(std::ostream& os, const T& t) const {
        default_formatter().element(os, t);
    }

    template <typename K, typename C, typename A>
        void prefix(std::ostream& os, const std::set<K, C, A>& s) const {

        os << "[" << s.size() << "]{";
    }

    template <typename T, typename A>
        void    prefix(std::ostream& os, const std::forward_list<T, A>&) const { os << "<"; }
    template <typename T, typename A>
        void separator(std::ostream& os, const std::forward_list<T, A>&) const { os << "->"; }
    template <typename T, typename A>
        void    suffix(std::ostream& os, const std::forward_list<T, A>&) const { os << ">"; }

    template <typename Ch, typename Tr, typename Al>
        void element(std::ostream& os, const std::basic_string<Ch, Tr, Al>& s) const {

        os << s;
    }
};


int main() {
    cout << "Empty vector: ";
    cout << vector<int>() << endl;

    cout << "Empty    set: ";
    cout << set<int>() << endl;

    cout << "Empty  std::tuple: ";
    cout << std::tuple<>() << endl;

    cout << "One-element vector: ";
    cout << vector<int>(1, 1701) << endl;

    {
        cout << "One-element    set: ";
        set<int> s;
        s.insert(1729);
        cout << s << endl;
    }

    {
        cout << "One-element  array: ";
        const int a[] = { 2048 };
        cout << a << endl;
    }

    cout << "One-element  std::tuple: ";
    cout << std::tuple<int>(4096) << endl;

    {
        cout << "Multi-element vector: ";
        vector<int> v;
        v.push_back(11);
        v.push_back(22);
        v.push_back(33);
        cout << v << endl;
    }

    {
        cout << "Multi-element    set: ";
        set<int> s;
        s.insert(111);
        s.insert(777);
        s.insert(222);
        s.insert(999);
        cout << s << endl;
    }

    {
        cout << "Multi-element  array: ";
        const int a[] = { 100, 200, 300, 400, 500 };
        cout << a << endl;
    }

    cout << "  Two-element   pair: ";
    cout << make_pair(123, 456) << endl;

    cout << "Multi-element  std::tuple: ";
    cout << make_tuple(10, 20, 30, 40) << endl;

    cout << "          Empty string: ";
    cout << string("") << endl;

    cout << "  One-character string: ";
    cout << string("x") << endl;

    cout << "Multi-character string: ";
    cout << string("meow") << endl;

    cout << "--" << endl;

    {
        cout << "vector<string>: ";
        vector<string> v;
        v.push_back("cute");
        v.push_back("fluffy");
        v.push_back("kittens");
        cout << v << endl;
    }

    {
        cout << "vector<vector<int>>: ";
        vector<vector<int>> v;
        for (int i = 0; i < 3; ++i) {
            vector<int> temp;

            for (int j = 0; j < 4; ++j) {
                temp.push_back((i + 1) * 10 + j);
            }

            v.push_back(temp);
        }
        cout << v << endl;

        cout << "map<string, vector<int>>: ";
        map<string, vector<int>> m;
        m["abc"] = v[0];
        m["def"] = v[1];
        m["ghi"] = v[2];
        cout << m << endl;
    }

    {
        cout << "Multi-dimensional array: ";
        const int aa[3][5] = {
            { 71, 72, 73, 74, 75 },
            { 81, 82, 83, 84, 85 },
            { 91, 92, 93, 94, 95 }
        };
        cout << aa << endl;
    }

    {
        cout << "vector<std::tuple<int, string, int>>: ";
        vector<std::tuple<int, string, int>> v;
        v.push_back(make_tuple(1, "ten", 100));
        v.push_back(make_tuple(2, "twenty", 200));
        v.push_back(make_tuple(3, "thirty", 300));
        cout << v << endl;
    }

    cout << endl << "*** special_formatter: ***" << endl;

    {
        vector<set<string>> v(3);
        v[0].insert("the");
        v[0].insert("wrath");
        v[0].insert("of");
        v[0].insert("khan");
        v[1].insert("the");
        v[1].insert("voyage");
        v[1].insert("home");
        v[2].insert("the");
        v[2].insert("undiscovered");
        v[2].insert("country");
        print_line(cout, v, special_formatter());
    }

    {
        set<pair<int, int>> s;
        s.insert(make_pair(11, 22));
        s.insert(make_pair(33, 44));
        s.insert(make_pair(55, 66));
        print_line(cout, s, special_formatter());
    }

    {
        forward_list<int> fl;
        fl.push_front(123);
        fl.push_front(456);
        fl.push_front(789);
        print_line(cout, fl, special_formatter());
    }

    {
        std::unique_ptr<int> u1;
        cout << u1 << endl;

        std::shared_ptr<int> u2;
        cout << u2 << endl;

        std::unique_ptr<int> u3(new int(35));
        cout << u3 << endl;

        std::shared_ptr<int> u4(new int(42));
        cout << u4 << endl;
    }

    getchar();
    return 0;
}
