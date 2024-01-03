#pragma once
#ifndef __TEST_HASH_AND_ALLOCATOR_HPP__
#define __TEST_HASH_AND_ALLOCATOR_HPP__

#include <string>

namespace Test {
    template<class T> // custom hash function
    class Hash {
    public:
        std::size_t operator()(T key) {
            return abs(static_cast<int>(key));
        }
    };

    template<>
    class Hash<std::string> {
    public:
        std::size_t operator()(std::string key) {
            return key.size();
        }
    };

    template<class T> // custom allocator
    class Allocator {
    public:
        using value_type = T;

        Allocator() noexcept {}
        template <typename U>
        Allocator(const Allocator<U>&) noexcept {}

        T* allocate(std::size_t n) { 
            return static_cast<T*>(::operator new(n * sizeof(T))); 
        }

        void deallocate(T* p, std::size_t n) { 
            ::operator delete(p, n * sizeof(T)); 
        }
    };
}

#endif // !__TEST_HASH_AND_ALLOCATOR_HPP__