#pragma once

#include <cassert>
#include <memory>

namespace VT
{
    namespace d_
    {
        enum Color { R, B };
    }

    // 1. No red node has a red child.
    // 2. Every path from root to empty node contains the same
    // number of black nodes.

    template<class T>
    class PTree
    {
    public:
        PTree() {}

        PTree(d_::Color c, PTree const & lft, T val, PTree const & rgt)
            : _root(std::make_shared<const Node>(c, lft._root, val, rgt._root))
        {
            assert(lft.isEmpty() || lft.root() < val);
            assert(rgt.isEmpty() || val < rgt.root());
        }

        bool isEmpty() const { return !_root; }

        T root() const
        {
            assert(!isEmpty());
            return _root->_val;
        }

        PTree left() const
        {
            assert(!isEmpty());
            return PTree(_root->_lft);
        }

        PTree right() const
        {
            assert(!isEmpty());
            return PTree(_root->_rgt);
        }

        bool member(T x) const
        {
            if (isEmpty())
                return false;

            T y = root();

            if (x < y)
                return left().member(x);
            else if (y < x)
                return right().member(x);
            else
                return true;
        }

        PTree insert(T x) const
        {
            PTree t = ins(x);
            return PTree(d_::B, t.left(), t.root(), t.right());
        }

        // 1. No red node has a red child.
        void assert1() const
        {
            if (!isEmpty())
            {
                auto lft = left();
                auto rgt = right();

                if (rootColor() == d_::R)
                {
                    assert(lft.isEmpty() || lft.rootColor() == d_::B);
                    assert(rgt.isEmpty() || rgt.rootColor() == d_::B);
                }

                lft.assert1();
                rgt.assert1();
            }
        }

        // 2. Every path from root to empty node contains the same
        // number of black nodes.
        int countB() const
        {
            if (isEmpty())
                return 0;

            int lft = left().countB();
            int rgt = right().countB();
            assert(lft == rgt);
            return (rootColor() == d_::B)? 1 + lft: lft;
        }

    private:
        struct Node
        {
            Node(d_::Color c,
                 std::shared_ptr<const Node> const & lft,
                 T val,
                 std::shared_ptr<const Node> const & rgt)
                : _c(c), _lft(lft), _val(val), _rgt(rgt)
            {}

            d_::Color _c;
            std::shared_ptr<const Node> _lft;
            T _val;
            std::shared_ptr<const Node> _rgt;
        };

        explicit PTree(std::shared_ptr<const Node> const & node) : _root(node) {}

        d_::Color rootColor() const
        {
            assert(!isEmpty());
            return _root->_c;
        }

        PTree ins(T x) const
        {
            assert1();

            if (isEmpty())
                return PTree(d_::R, PTree(), x, PTree());

            T y = root();
            d_::Color c = rootColor();

            if (rootColor() == d_::B)
            {
                if (x < y)
                    return balance(left().ins(x), y, right());
                else if (y < x)
                    return balance(left(), y, right().ins(x));
                else
                    return *this; // no duplicates
            }
            else
            {
                if (x < y)
                    return PTree(c, left().ins(x), y, right());
                else if (y < x)
                    return PTree(c, left(), y, right().ins(x));
                else
                    return *this; // no duplicates
            }
        }

        // Called only when parent is black
        static PTree balance(PTree const & lft, T x, PTree const & rgt)
        {
            if (lft.doubledLeft())
                return PTree(d_::R
                            , lft.left().paint(d_::B)
                            , lft.root()
                            , PTree(d_::B, lft.right(), x, rgt));
            else if (lft.doubledRight())
                return PTree(R
                            , PTree(d_::B, lft.left(), lft.root(), lft.right().left())
                            , lft.right().root()
                            , PTree(d_::B, lft.right().right(), x, rgt));
            else if (rgt.doubledLeft())
                return PTree(d_::R
                            , PTree(d_::B, lft, x, rgt.left().left())
                            , rgt.left().root()
                            , PTree(d_::B, rgt.left().right(), rgt.root(), rgt.right()));
            else if (rgt.doubledRight())
                return PTree(d_::R
                            , PTree(d_::B, lft, x, rgt.left())
                            , rgt.root()
                            , rgt.right().paint(d_::B));
            else
                return PTree(d_::B, lft, x, rgt);
        }

        bool doubledLeft() const
        {
            return !isEmpty()
                && rootColor() == d_::R
                && !left().isEmpty()
                && left().rootColor() == d_::R;
        }

        bool doubledRight() const
        {
            return !isEmpty()
                && rootColor() == d_::R
                && !right().isEmpty()
                && right().rootColor() == d_::R;
        }

        PTree paint(d_::Color c) const
        {
            assert(!isEmpty());
            return PTree(c, left(), root(), right());
        }

        std::shared_ptr<const Node> _root;
    };

    template<class T, class F>
    void forEach(const PTree<T>& t, F f)
    {
        if (!t.isEmpty())
        {
            forEach(t.left(), f);
            f(t.root());
            forEach(t.right(), f);
        }
    }

    template<class T, class Beg, class End>
    PTree<T> insert(PTree<T> t, Beg it, End end)
    {
        if (it == end)
            return t;

        T item = *it;
        auto t1 = insert(t, ++it, end);
        return t1.insert(item);
    }

}
