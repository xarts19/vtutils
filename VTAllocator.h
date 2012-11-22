#pragma once

#include <list>
#include <memory>
#include <limits>

#include <cassert>

#define UNUSED( x ) ( &reinterpret_cast< const int& >( x ) )

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

    // Segregated storage allocator
    template <typename T>
    struct SSAllocator
    {
        typedef T               value_type;
        typedef T*              pointer;
        typedef const T*        const_pointer;
        typedef T&              reference;
        typedef const T&        const_reference;
        typedef std::size_t     size_type;
        typedef std::ptrdiff_t  difference_type;
        
        template<typename U>
        struct rebind
        {
            typedef SSAllocator<U> other;
        };

        pointer address (reference value) const
        {
            return &value;
        }

        const_pointer address (const_reference value) const
        {
            return &value;
        }

        /* constructors and destructor
           nothing to do because the allocator has no state
        */
        SSAllocator() { }
        SSAllocator(const SSAllocator&) { }
        template <class U>
        SSAllocator (const SSAllocator<U>&) { }
        ~SSAllocator() { }

        // return maximum number of elements that can be allocated
        size_type max_size () const
        {
            return std::numeric_limits<std::size_t>::max() / sizeof(T);
        }

        pointer allocate(size_type n, SSAllocator<void>::const_pointer p = 0)
        {
            UNUSED(p);
            assert(n <= 1);
            return (pointer)(::operator new(n * sizeof(T)));
        }

        void deallocate(pointer p, size_type n)
        {
            UNUSED(n);
            ::operator delete((void*)p);
        }

        void construct(pointer p, const T& t)
        {
            new( (void*)p ) T(t);
        }

        void destroy(pointer p)
        {
            UNUSED(p);
            p->~T();
        }

    private:
        void operator=(const SSAllocator&);
    };

    template <>
    struct SSAllocator<void>
    {
        typedef void        value_type;
        typedef void*       pointer;
        typedef const void* const_pointer;

        template <class U> 
        struct rebind
        {
            typedef SSAllocator<U> other;
        };
    };

    template< typename T, typename U >
    bool operator==( const SSAllocator<T>& a, const SSAllocator<U>& b )
    { return true; }

    template< typename T, typename U >
    bool operator!=( const SSAllocator<T>& a, const SSAllocator<U>& b )
    { return false; }


};
