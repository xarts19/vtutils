#pragma once

#include <list>

namespace VT
{

    template <typename T>
    class Allocator
    {
    public:
        Allocator(unsigned int block_size, unsigned int capacity) : 
            buffer_(static_cast<char*>(::malloc(capacity * block_size))),
            block_size_(block_size)
        {
            if (buffer_ == NULL)
                return;
            for (unsigned int i = 0; i < capacity; ++i)
                free_blocks_.push_back(buffer_ + i * block_size_);
        }

        ~Allocator()
        {
            ::free(buffer_);
        }

        T* alloc()
        {
            if (free_blocks_.empty())
                return NULL;
            char* ptr = free_blocks_.front();
            free_blocks_.pop_front();
            return reinterpret_cast<T*>(ptr);
        }

        void free(T* ptr)
        {
            assert( (reinterpret_cast<char*>(ptr) - buffer_) % block_size_ == 0 );      //check that ptr is correctly aligned
            free_blocks_.push_back(reinterpret_cast<char*>(ptr));
        }

        bool is_valid() const { return buffer_ != NULL; }

    private:
        char* buffer_;
        const unsigned int block_size_;
        std::list<char*> free_blocks_;
    };


    namespace detail_
    {
        class PoolChunk
        {
            
        };
    }

};
