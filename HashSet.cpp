#include "Vector.hpp" // I use My::Vector to implement iterators
#include <algorithm>
#include <string>
#include <functional>
#include "TestHashAndAllocator.hpp"

namespace My {
    template <typename T, typename Hash = std::hash<T>, typename Allocator = std::allocator<T>>
    class HashSet {
    private:
        const std::size_t DEFAULT_NUMBER_OF_BUCKETS = 8;
        const std::size_t FACTOR_OF_REHASHING = 2;
        const float REHASHING_COEFFICIENT = 0.7f;
        
        enum class BucketState { ABSENT, PRESENT, DELETED };

        T* table;
        BucketState* flag;
        Allocator alloc;
        std::allocator<BucketState> state_alloc;
        Hash hash;

        std::size_t number_of_buckets;
        std::size_t buckets_used;

        void rehash();
        void create_new_table(const T& key);

        My::Vector<T> iter_vec;

    public:
        HashSet(const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashSet(int size, const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashSet(std::initializer_list<T> init_list, const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashSet(const HashSet& other);
        HashSet(HashSet&& other) noexcept;

        ~HashSet();

        HashSet& operator = (const HashSet& other);
        HashSet& operator = (HashSet&& other) noexcept;

        void insert(const T& key);
        void erase(const T& key);
        void clear();
        std::size_t size() const noexcept;
        std::size_t bucket_count() const noexcept;
        bool empty() const noexcept;
        int bucket(const T& key) const noexcept;
        bool count(const T& key) const noexcept;
        void display() const; // additional method to display hash-table and bucket status, this works only with primitive data types

        class iterator {
            typename My::Vector<T>::iterator ptr;
        public:
            iterator() = default;
            iterator(typename My::Vector<T>::iterator _ptr) : ptr(_ptr) {}
            const T& operator* () { return *ptr; }
            const typename My::Vector<T>::iterator operator-> () { return ptr; }
            iterator& operator++ () { ++ptr; return *this; }
            iterator operator++ (int) { iterator tmp = *this; ++* this; return tmp; }
            iterator& operator-- () { --ptr; return *this; }
            iterator operator-- (int) { iterator tmp = *this; --* this; return tmp; }
            bool operator== (const iterator& it) { return ptr == it.ptr; }
            bool operator!= (const iterator& it) { return !(*this == it); }
        };

        iterator begin() { return iterator(iter_vec.begin()); }
        iterator end() { return iterator(iter_vec.end()); }
    };

    template <typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::HashSet(const Hash& _hash, const Allocator& _alloc) : hash(_hash), alloc(_alloc) {
        number_of_buckets = DEFAULT_NUMBER_OF_BUCKETS;
        buckets_used = 0;

        table = alloc.allocate(number_of_buckets);
        flag = state_alloc.allocate(number_of_buckets);
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::construct(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i);
        }
    }

    template <typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::HashSet(int size, const Hash& _hash, const Allocator& _alloc) : hash(_hash), alloc(_alloc) {
        if (size <= 0) throw std::length_error("Table size must be greater then 0."); // EXCEPTION
        number_of_buckets = size;
        buckets_used = 0;

        table = alloc.allocate(number_of_buckets);
        flag = state_alloc.allocate(number_of_buckets);
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::construct(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i);
        }
    }

    template<typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::HashSet(std::initializer_list<T> init_list, const Hash& _hash, const Allocator& _alloc) : HashSet<T, Hash, Allocator>::HashSet(init_list.size(), _hash, _alloc) {
        for (auto& el : init_list) {
            insert(el);
        }
    }

    template <typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::HashSet(const HashSet& other) {
        number_of_buckets = other.number_of_buckets;
        buckets_used = other.buckets_used;
        hash = other.hash;
        alloc = other.alloc;
        iter_vec = other.iter_vec;
        if (other.table && other.flag) {
            table = alloc.allocate(number_of_buckets);
            flag = state_alloc.allocate(number_of_buckets);
            for (std::size_t i = 0; i < number_of_buckets; i++) {
                std::allocator_traits<Allocator>::construct(alloc, table + i, other.table[i]);
                std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i, other.flag[i]);
            }
        }
        else {
            table = nullptr;
            flag = nullptr;
        }
    }

    template<typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::HashSet(HashSet&& other) noexcept {
        number_of_buckets = other.number_of_buckets;
        buckets_used = other.buckets_used;
        table = other.table;
        flag = other.flag;
        hash = std::move(other.hash);
        alloc = std::move(other.alloc);
        iter_vec = std::move(other.iter_vec);

        other.number_of_buckets = 0;
        other.buckets_used = 0;
        other.table = nullptr;
        other.flag = nullptr;
    }

    template <typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>::~HashSet() {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + i);
        }
        alloc.deallocate(table, number_of_buckets);
        state_alloc.deallocate(flag, number_of_buckets);
    }

    template <typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>& HashSet<T, Hash, Allocator>::operator = (const HashSet& other) {
        if (this != &other) {
            for (std::size_t i = 0; i < number_of_buckets; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, table + i);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + i);
            }
            alloc.deallocate(table, number_of_buckets);
            state_alloc.deallocate(flag, number_of_buckets);

            number_of_buckets = other.number_of_buckets;
            buckets_used = other.buckets_used;
            hash = other.hash;
            alloc = other.alloc;
            iter_vec = other.iter_vec;
            if (other.table && other.flag) {
                table = alloc.allocate(number_of_buckets);
                flag = state_alloc.allocate(number_of_buckets);
                for (std::size_t i = 0; i < number_of_buckets; i++)
                {
                    std::allocator_traits<Allocator>::construct(alloc, table + i, other.table[i]);
                    std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i, other.flag[i]);
                }
            }
            else {
                table = nullptr;
                flag = nullptr;
            }
        }
        return *this;
    }

    template<typename T, typename Hash, typename Allocator>
    HashSet<T, Hash, Allocator>& HashSet<T, Hash, Allocator>::operator =(HashSet&& other) noexcept {
        if (this != &other) {
            for (std::size_t i = 0; i < number_of_buckets; i++) {
                std::allocator_traits<Allocator>::destroy(alloc, table + i);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + i);
            }
            alloc.deallocate(table, number_of_buckets);
            state_alloc.deallocate(flag, number_of_buckets);

            number_of_buckets = other.number_of_buckets;
            buckets_used = other.buckets_used;
            table = other.table;
            flag = other.flag;
            hash = std::move(other.hash);
            alloc = std::move(other.alloc);
            iter_vec = std::move(other.iter_vec);

            other.number_of_buckets = 0;
            other.buckets_used = 0;
            other.table = nullptr;
            other.flag = nullptr;
        }
        return *this;
    }

    template <typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::rehash() {
        T* copy_of_table = std::move(table);
        BucketState* copy_of_flag = std::move(flag);
        std::size_t old_number_of_buckets = number_of_buckets;
        number_of_buckets = number_of_buckets * FACTOR_OF_REHASHING;

        table = alloc.allocate(number_of_buckets);
        flag = state_alloc.allocate(number_of_buckets);
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::construct(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i);
        }

        for (std::size_t i = 0; i < old_number_of_buckets; i++) {
            if (copy_of_flag[i] == BucketState::DELETED) {
                std::allocator_traits<Allocator>::destroy(alloc, copy_of_flag + i);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, copy_of_flag + i);
                buckets_used--;
            }
            if (copy_of_flag[i] == BucketState::DELETED) {
                std::allocator_traits<Allocator>::construct(alloc, copy_of_flag + i);
                std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, copy_of_flag + i, BucketState::ABSENT);
            }
            if (copy_of_flag[i] == BucketState::PRESENT)
                create_new_table(copy_of_table[i]);
        }

        for (std::size_t i = 0; i < old_number_of_buckets; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, copy_of_table + i);
            std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, copy_of_flag + i);
        }
        alloc.deallocate(copy_of_table, old_number_of_buckets);
        state_alloc.deallocate(copy_of_flag, old_number_of_buckets);
    }

    template<typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::create_new_table(const T& key) {
        std::size_t index = hash(key) % number_of_buckets;
        while (true) {
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::destroy(alloc, table + index);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + index);
            }
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::construct(alloc, table + index, key);
                std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + index, BucketState::PRESENT);
                break;
            }
            else {
                index++;
                if (index >= number_of_buckets) {
                    index = 0;
                }
            };
        }
    }

    template<typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::insert(const T& key) {
        std::size_t index = hash(key) % number_of_buckets;
        while (true) {
            if (table[index] == key && flag[index] == BucketState::PRESENT) {
                break;
            }
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::destroy(alloc, table + index);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + index);
            }
            if (flag[index] == BucketState::ABSENT) {
                buckets_used++;
                std::allocator_traits<Allocator>::construct(alloc, table + index, key);
                std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + index, BucketState::PRESENT);
                iter_vec.push_back(key);
                break;
            }
            else {
                index++;
                if (index >= number_of_buckets) {
                    index = 0;
                }
            };
        }
        if (static_cast<float>(buckets_used / number_of_buckets) >= REHASHING_COEFFICIENT) {
            rehash();
        }
    }

    template<typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::erase(const T& key) {
        std::size_t index = hash(key) % number_of_buckets;
        if (table[index] != key) {
            do {
                index++;
                if (index >= number_of_buckets) {
                    index = 0;
                }
            } while (table[index] != key && flag[index] != BucketState::ABSENT);
        }
        if (table[index] == key && flag[index] != BucketState::DELETED) {
            flag[index] = BucketState::DELETED;
            iter_vec.erase(std::remove_if(iter_vec.begin(), iter_vec.end(), [key](T i) {return i == key; }));
        }
    }

    template<typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::clear() {
        iter_vec.clear();

        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + i);
        }
        alloc.deallocate(table, number_of_buckets);
        state_alloc.deallocate(flag, number_of_buckets);

        number_of_buckets = DEFAULT_NUMBER_OF_BUCKETS;
        buckets_used = 0;

        table = alloc.allocate(number_of_buckets);
        flag = state_alloc.allocate(number_of_buckets);
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::construct(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i);
        }
    }

    template<typename T, typename Hash, typename Allocator>
    std::size_t HashSet<T, Hash, Allocator>::size() const noexcept { return iter_vec.size(); }

    template<typename T, typename Hash, typename Allocator>
    std::size_t HashSet<T, Hash, Allocator>::bucket_count() const noexcept { return number_of_buckets; }

    template<typename T, typename Hash, typename Allocator>
    bool HashSet<T, Hash, Allocator>::empty() const noexcept { return size() == 0; }

    template<typename T, typename Hash, typename Allocator>
    int HashSet<T, Hash, Allocator>::bucket(const T& key) const noexcept {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            if (table[i] == key && flag[i] == BucketState::PRESENT) return i;
        }
        return -1;
    }

    template<typename T, typename Hash, typename Allocator>
    bool HashSet<T, Hash, Allocator>::count(const T& key) const noexcept {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            if (table[i] == key && flag[i] == BucketState::PRESENT) return true;
        }
        return false;
    }

    template<typename T, typename Hash, typename Allocator>
    void HashSet<T, Hash, Allocator>::display() const {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::cout << i << ": key: " << table[i] << "; flag: " << static_cast<int>(flag[i]) << std::endl;
        }
    }
}

int main() {
    using namespace My;

    std::cout << "My::HashSet\n";

    HashSet<int> A;
    A.insert(1);
    A.erase(1);
    A.insert(2);
    A.erase(2);
    A.insert(3);
    A.insert(4);
    A.insert(5);
 
    for (auto& i : A)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::cout << "A.size(): " << A.size() << "\nA.bucket_count(): " << A.bucket_count() << "\n";

    HashSet<int> B = A;

    A = B;

    for (HashSet<int>::iterator it = B.begin(); it != B.end(); it++)
    {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    B.insert(9);
    B.insert(53);
    B.insert(979);
    B.insert(32);

    A = std::move(B);

    for (auto& i : A)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    A.clear();
    std::cout << A.empty() << "\n\n";

    std::cout << "initializer_list\n";

    HashSet<std::string> E{"Apple", "Banana", "Cucumber", "Frog", "Set", "31314", " ", "ABCDE"};
    E.erase("31314");

    for (auto& i : E)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    std::cout << E.size() << "\n\n";

    std::cout << "custom hash function + custom allocator\n";

    HashSet <int, Test::Hash<int>, Test::Allocator<int>> G;

    G.insert(1);
    G.insert(20);
    G.erase(20);

    G.display();

    return 0;
}