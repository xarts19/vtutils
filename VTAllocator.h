#pragma once

// TODO:
//  * add the ability to allocate arrays of objects to Pool

#include <list>
#include <memory>
#include <limits>

#include <cassert>
#include <stdlib.h>

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

    private:
        Allocator(const Allocator&);
        Allocator& operator=(const Allocator&);
    };


    class Pool
    {
    private:
        struct Link
        {
            Link* next;
        };
    
        struct Chunk
        {
            enum {size = 8 * 1024 - 16};
            Chunk* next;
            char mem[size];
        };

    private:
        Link* head;
        Chunk* chunks;
        const unsigned int esize;
        unsigned int allocated_;
    
    private:
        Pool(Pool&);            //copy protection
        void operator=(Pool&);  //copy protection

        // make pool larger
        void grow()    // allocate new ‘chunk’, organize it as a linked list of elements of size ’esize’
        {
            Chunk* n = new Chunk;
            n->next = chunks;
            chunks = n;
    
            const unsigned int nelem = Chunk::size / esize;
            char* start = n->mem;
            char* last = &start[(nelem - 1) * esize];
    
            for (char* p = start; p < last; p += esize)     // assume sizeof(Link) <= esize
                reinterpret_cast<Link*>(p)->next = reinterpret_cast<Link*>(p + esize);
            reinterpret_cast<Link*>(last)->next = 0;
            head = reinterpret_cast<Link*>(start);
        }

        void clear()
        {
            Chunk* n = chunks;
            while (n)
            {
                Chunk* p = n;
                n = n->next;
                delete p;
            }
            head = NULL;
            chunks = NULL;
        }

    public:
        Pool(unsigned int n) :        // n is the size of elements
            head(NULL),
            chunks(NULL),
            esize(n < sizeof(Link*) ? sizeof(Link*) : n)
        { assert(n < Chunk::size); }

        ~Pool()       // free all chunks
        {
            clear();
        }
    
        inline void* alloc()          // allocate one element
        {
            if (head == NULL) grow();
            Link* p = head;         // return first element
            head = p->next;
            ++allocated_;
            return p;
        }

        inline void free(void* b)      // put an element back into the pool
        {
            assert(allocated_ > 0);
            Link* p = static_cast<Link*>(b);
            p->next = head;              // put b back as first element
            head = p;
            if (--allocated_ == 0) clear();
        }
    };


    // Segregated storage allocator
    template <typename T>
    struct SSAllocator
    {
    private:
        static Pool pool_;      // pool of elements of sizeof(T)

    public:
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

        pointer address(reference value) const
        {
            return &value;
        }

        const_pointer address(const_reference value) const
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
        size_type max_size() const
        {
            return std::numeric_limits<std::size_t>::max() / sizeof(T);
        }

        pointer allocate(size_type n, SSAllocator<void>::const_pointer p = 0)
        {
            UNUSED(p);
            assert(n == 1);
            return static_cast<pointer>(pool_.alloc());
        }

        void deallocate(pointer p, size_type n)
        {
            assert(n == 1);
            pool_.free(p);
        }

        void construct(pointer p)
        {
            new (p) T();
        }

        void construct(pointer p, const T& t)
        {
            new (p) T(t);
        }

        template <typename... Args>
        void construct(pointer p, Args&&... args)
        {
            new (p) T(std::forward<Args>(args)...);
        }

        void destroy(pointer p)
        {
            UNUSED(p);
            p->~T();
        }

    private:
        void operator=(const SSAllocator&);
    };

    template <typename T> VT::Pool VT::SSAllocator<T>::pool_(sizeof(T));


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
    bool operator==( const SSAllocator<T>&, const SSAllocator<U>& )
    { return true; }

    template< typename T, typename U >
    bool operator!=( const SSAllocator<T>&, const SSAllocator<U>& )
    { return false; }


};
