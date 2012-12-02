#pragma once

#include <algorithm>
#include <vector>
#include <utility>

namespace VT
{

    template <typename KeyType, typename ValType, 
        typename AllocType = std::allocator<std::pair<KeyType, ValType>>>
    class SortedVectorMap
    {
    private:
        typedef std::pair<KeyType, ValType>         ElemType;
        typedef std::vector<ElemType, AllocType>    VectorType;
        
        VectorType vector_;

    private:
        class Compare
        {
        public:
            bool operator()(const ElemType& lhs, const ElemType& rhs)
            { return lhs.first < rhs.first; }
            bool operator()(const ElemType& lhs, const KeyType& key)
            { return lhs.first < key; }
            bool operator()(const KeyType& key, const ElemType& rhs)
            { return key < rhs.first; }
        };

    public:
        typedef typename VectorType::iterator        iterator;
        typedef typename VectorType::const_iterator  const_iterator;

        typedef std::size_t size_type;

    private:
        SortedVectorMap(const SortedVectorMap&);
        SortedVectorMap& operator=(const SortedVectorMap&);

    public:
        SortedVectorMap() : vector_() { }

        void insert(const ElemType& value)
        {
            vector_.push_back(value);
        }

        template <typename... Args>
        std::pair<iterator, bool> emplace(Args... args)
        {
            vector_.emplace_back(std::forward<Args>(args)...);
            return std::pair<iterator, bool>(vector_.begin() + vector_.size(), true);
        }

        void reserve(size_type n)
        {
            vector_.reserve(n);
        }

        iterator find(const KeyType& key)
        {
            return lower_bound(vector_.begin(), vector_.end(), key, Compare());
        }

        const_iterator find(const KeyType& key) const
        {
            return lower_bound(vector_.begin(), vector_.end(), key, Compare());
        }

        iterator end()
        {
            return vector_.end();
        }

        const_iterator end() const
        {
            return vector_.end();
        }

        size_type count(const KeyType& key) const
        {
            std::pair<typename VectorType::iterator, typename VectorType::iterator> bounds;
            bounds = equal_range(vector_.begin(), vector_.end(), key, Compare());
            return bounds.second - bounds.first;
        }

        void sort()
        {
            std::sort(vector_.begin(), vector_.end(), Compare());
        }
    };

}

