// This file is created as a header-only library to implement iterators for My::HashSet ans My::HashMap

#pragma once
#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include <iostream>
#include <stdexcept>
#include <utility>
#include <initializer_list>
#include <memory>

namespace My {
    template<typename T, typename Allocator = std::allocator<T>>
    class Vector {
        std::size_t cp; // capacity
        std::size_t sz; // size
        T* data;
        Allocator alloc;

    public:
        class iterator {
            T* ptr;
        public:
            iterator() = default;
            iterator(T* _ptr) : ptr(_ptr) {}
            T& operator*() const noexcept { return *ptr; }
            T* operator->() const noexcept { return ptr; }
            bool operator==(const iterator& second) const noexcept { return ptr == second.ptr; }
            bool operator!=(const iterator& second) const noexcept { return !(*this == second); }
            bool operator<(const iterator& second) const noexcept { return ptr < second.ptr; }
            bool operator>(const iterator& second) const noexcept { return second < *this; }
            bool operator<=(const iterator& second) const noexcept { return !(second < *this); }
            bool operator>=(const iterator& second) const noexcept { return !(*this < second); }
            iterator& operator++() noexcept { ++ptr; return *this; }
            iterator operator++(int) noexcept { iterator tmp = *this; ++* this; return tmp; }
            iterator& operator--() noexcept { --ptr; return *this; };
            iterator operator--(int) noexcept { iterator tmp = *this; --* this; return tmp; };
            iterator operator+(int offset) const noexcept { iterator tmp = *this; tmp.ptr += offset; return tmp; }
            iterator operator-(int offset) const noexcept { iterator tmp = *this; tmp.ptr -= offset; return tmp; }
            iterator& operator+=(int offset) noexcept { ptr += offset; return *this; }
            iterator& operator-=(int offset) noexcept { ptr -= offset; return *this; }
        };

        Vector(const Allocator& _alloc = Allocator());
        Vector(int size, const Allocator& _alloc = Allocator());
        Vector(std::initializer_list<T> init_list, const Allocator& _alloc = Allocator());
        Vector(std::size_t size, const T& value, const Allocator& _alloc = Allocator());
        Vector(const Vector& other);
        Vector(Vector&& other) noexcept;

        ~Vector();

        Vector& operator =(const Vector& other);
        Vector& operator =(Vector&& other) noexcept;
        T& operator[](std::size_t index) const;

        void push_back(const T& element);
        void pop_back();
        iterator insert(iterator position, const T& element);
        iterator insert(iterator position, int number, const T& element);
        iterator insert(iterator position, std::initializer_list<T> init_list);
        iterator erase(iterator first);
        iterator erase(iterator first, iterator second);
        std::size_t size() const noexcept;
        std::size_t capacity() const noexcept;
        void resize(int size);
        void reserve(int capacity);
        void clear();
        bool empty() const noexcept;
        T& front() const;
        T& back() const;
        T& at(std::size_t index) const;

        iterator begin() { return iterator(data); };
        iterator end() { return iterator(data + sz); };
    };

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(const Allocator& _alloc) : sz(0), cp(1), alloc(_alloc) {
        data = alloc.allocate(cp);
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(int size, const Allocator& _alloc) : sz(size), cp(size), alloc(_alloc) {
        if (size < 0) throw std::length_error("size must be greater than 0."); // EXCEPTION
        if (sz == 0) cp++;

        data = alloc.allocate(cp);
        for (std::size_t i = 0; i < sz; i++) {
            std::allocator_traits<Allocator>::construct(alloc, data + i);
        }
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(std::initializer_list<T> init_list, const Allocator& _alloc) : sz(init_list.size()), cp(init_list.size()), alloc(_alloc) {
        data = alloc.allocate(cp);
        std::size_t index = 0;
        for (auto& el : init_list) {
            std::allocator_traits<Allocator>::construct(alloc, data + index++, el);
        }
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(std::size_t size, const T& value, const Allocator& _alloc) : sz(size), cp(size), alloc(_alloc) {
        data = alloc.allocate(cp);
        for (std::size_t i = 0; i < sz; i++) {
            std::allocator_traits<Allocator>::construct(alloc, data + i, value);
        }
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(const Vector& other) : sz(other.sz), cp(other.cp), alloc(other.alloc) {
        if (other.data) {
            data = alloc.allocate(cp);
            for (std::size_t i = 0; i < sz; i++) {
                std::allocator_traits<Allocator>::construct(alloc, data + i, other.data[i]);
            }
        }
        else {
            data = nullptr;
        }
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::Vector(Vector&& other) noexcept : sz(other.sz), cp(other.cp), data(other.data), alloc(std::move(other.alloc)) { other.sz = 0, other.cp = 0, other.data = nullptr; }

    template<typename T, typename Allocator>
    Vector<T, Allocator>::~Vector() {
        clear();
        alloc.deallocate(data, cp);
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>& Vector<T, Allocator>::operator =(const Vector& other) {
        if (this != &other) {
            for (std::size_t i = 0; i < sz; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, data + i);
            }
            alloc.deallocate(data, cp);

            cp = other.cp;
            sz = other.sz;
            alloc = other.alloc;
            if (other.data) {
                data = alloc.allocate(cp);
                for (std::size_t i = 0; i < sz; i++) {
                    std::allocator_traits<Allocator>::construct(alloc, data + i, other.data[i]);
                }
            }
            else {
                data = nullptr;
            }
        }
        return *this;
    }

    template<typename T, typename Allocator>
    Vector<T, Allocator>& Vector<T, Allocator>::operator=(Vector&& other) noexcept {
        if (this != &other) {
            for (std::size_t i = 0; i < sz; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, data + i);
            }
            alloc.deallocate(data, cp);

            sz = other.sz, cp = other.cp, data = other.data, alloc = std::move(other.alloc);
            other.sz = 0, other.cp = 0, other.data = nullptr;
        }
        return *this;
    }

    template<typename T, typename Allocator>
    T& Vector<T, Allocator>::operator[](std::size_t index) const {
        if (index < 0 || index >= sz) throw std::out_of_range("out of the range."); // EXCEPTION
        return data[index];
    }

    template<typename T, typename Allocator>
    void Vector<T, Allocator>::push_back(const T& element) {
        sz += 1;
        if (sz > cp) {
            std::size_t old_capacity = cp;
            cp *= 2;

            T* new_data = alloc.allocate(cp);
            for (std::size_t i = 0; i < old_capacity; i++) {
                std::allocator_traits<Allocator>::construct(alloc, new_data + i, data[i]);
            }
            for (std::size_t i = 0; i < old_capacity; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, data + i);
            }
            alloc.deallocate(data, old_capacity);
            data = new_data;
        }
        std::allocator_traits<Allocator>::construct(alloc, data + sz - 1, element);
    }

    template<typename T, typename Allocator>
    void Vector<T, Allocator>::pop_back() {
        if (sz == 0) throw std::out_of_range("vector empty before pop."); // EXCEPTION
        std::allocator_traits<Allocator>::destroy(alloc, data + --sz);
    }

    template<typename T, typename Allocator>
    typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(iterator position, const T& element) {
        if (position < begin() || position > end()) throw std::out_of_range("vector insert iterator outside range."); // EXCEPTION
        if (position == end()) {
            push_back(element);
            return end() - 1;
        }

        int offset = 0;
        for (iterator it = position; it != begin(); --it) {
            offset++;
        }
        T last = *(end() - 1);
        for (iterator it = end() - 1; it != position; --it) {
            *it = *(it - 1);
        }
        *position = element;
        push_back(last);
        return iterator(begin() + offset);
    }

    template<typename T, typename Allocator>
    typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(iterator position, int number, const T& element) {
        if (number < 0) throw std::length_error("length error."); // EXCEPTION
        iterator cur_pos = position;
        for (std::size_t i = number; i > 0; i--) {
            cur_pos = insert(cur_pos, element);
            cur_pos++;
        }
        return cur_pos - number;
    }

    template<typename T, typename Allocator>
    typename Vector<T, Allocator>::iterator Vector<T, Allocator>::insert(iterator position, std::initializer_list<T> init_list) {
        iterator cur_pos = position;
        for (auto& el : init_list) {
            cur_pos = insert(cur_pos, el);
            cur_pos++;
        }
        return cur_pos - init_list.size();
    }

    template<typename T, typename Allocator>
    typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase(iterator first) {
        if (first < begin() || first >= end()) throw std::out_of_range("vector erase iterator outside range."); // EXCEPTION
        for (iterator it = first; it != end() - 1; ++it) {
            *it = *(it + 1);
        }
        std::allocator_traits<Allocator>::destroy(alloc, data + --sz);
        return first;
    }

    template<typename T, typename Allocator>
    typename Vector<T, Allocator>::iterator Vector<T, Allocator>::erase(iterator first, iterator second) {
        if (first < begin() || first > end() || second < first || second > end()) throw std::out_of_range("vector erase iterator outside range."); // EXCEPTION
        if (first == second) return second;

        int offset = 0;
        for (iterator it = first; it != second; ++it) {
            offset++;
        }
        iterator cur_pos = first;
        for (std::size_t i = offset; i > 0; i--) {
            cur_pos = erase(cur_pos);
        }
        return first;
    }

    template<typename T, typename Allocator>
    std::size_t Vector<T, Allocator>::size() const noexcept { return sz; }

    template<typename T, typename Allocator>
    std::size_t Vector<T, Allocator>::capacity() const noexcept { return cp; }

    template<typename T, typename Allocator>
    void Vector<T, Allocator>::resize(int size) {
        if (size < 0) throw std::length_error("lenght error."); // EXCEPTION
        std::size_t old_size = sz;
        sz = size;
        if (sz > cp) {
            cp = sz;

            T* new_data = alloc.allocate(cp);
            for (std::size_t i = 0; i < old_size; i++) {
                std::allocator_traits<Allocator>::construct(alloc, new_data + i, data[i]);
            }
            for (std::size_t i = 0; i < old_size; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, data + i);
            }
            alloc.deallocate(data, cp);
            data = new_data;
        }
        for (std::size_t i = old_size; i < sz; i++) {
            std::allocator_traits<Allocator>::construct(alloc, data + i);
        }
    }

    template<typename T, typename Allocator>
    void Vector<T, Allocator>::reserve(int capacity) {
        if (capacity < 0) throw std::length_error("capacity error."); // EXCEPTION
        if (capacity > cp) {
            std::size_t old_capacity = cp;
            cp = capacity;

            T* new_data = alloc.allocate(cp);
            for (std::size_t i = 0; i < sz; i++) {
                std::allocator_traits<Allocator>::construct(alloc, new_data + i, data[i]);
            }
            for (std::size_t i = 0; i < sz; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, data + i);
            }
            alloc.deallocate(data, old_capacity);
            data = new_data;
        }
    }

    template<typename T, typename Allocator>
    void Vector<T, Allocator>::clear() {
        for (std::size_t i = 0; i < sz; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, data + i);
        }
        sz = 0;
    }

    template<typename T, typename Allocator>
    bool Vector<T, Allocator>::empty() const noexcept { return sz == 0; }

    template<typename T, typename Allocator>
    T& Vector<T, Allocator>::front() const { return data[0]; }

    template<typename T, typename Allocator>
    T& Vector<T, Allocator>::back() const { return data[sz - 1]; }

    template<typename T, typename Allocator>
    T& Vector<T, Allocator>::at(std::size_t index) const { return operator[](index); }

}

#endif // !__VECTOR_HPP__