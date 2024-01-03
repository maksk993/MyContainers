#include "Vector.hpp" // I use My::Vector to implement iterators
#include <algorithm>
#include <string>
#include <functional>
#include "TestHashAndAllocator.hpp"

namespace My {
    template <typename T1, typename T2, typename Hash = std::hash<T1>, typename Allocator = std::allocator<std::pair<T1, T2>>>
    class HashMap {
    private:
        const std::size_t DEFAULT_NUMBER_OF_BUCKETS = 8;
        const std::size_t FACTOR_OF_REHASHING = 2;
        const float REHASHING_COEFFICIENT = 0.7f;

        enum class BucketState { ABSENT, PRESENT, DELETED };

        std::pair<T1, T2>* table;
        BucketState* flag;
        Allocator alloc;
        std::allocator<BucketState> state_alloc;
        Hash hash;

        std::size_t number_of_buckets;
        std::size_t buckets_used;

        void rehash();
        void create_new_table(const T1& key, const T2& value);

        My::Vector<std::pair<T1, T2>> iter_vec;

    public:
        HashMap(const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashMap(int size, const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashMap(std::initializer_list<std::pair<T1, T2>> init_list, const Hash& _hash = Hash(), const Allocator& _alloc = Allocator());
        HashMap(const HashMap& other);
        HashMap(HashMap&& other) noexcept;

        ~HashMap();

        HashMap& operator = (const HashMap& other);
        HashMap& operator = (HashMap&& other) noexcept;
        T2& operator [](const T1& key);

        void insert(const T1& key, const T2& value);
        void insert(std::pair<T1, T2> pair_key_value);
        void erase(const T1& key);
        T2& at(const T1& key);
        void clear();
        std::size_t size() const noexcept;
        std::size_t bucket_count() const noexcept;
        bool empty() const noexcept;
        int bucket(const T1& key) const noexcept;
        bool count(const T1& key) const noexcept;
        void display() const; // additional method to display hash-table and bucket status, works only with primitive data types

        class iterator {
            typename My::Vector<std::pair<T1, T2>>::iterator ptr;
            HashMap<T1, T2>* this_map;
        public:
            iterator() = default;
            iterator(typename My::Vector<std::pair<T1, T2>>::iterator _ptr, HashMap<T1, T2>* _this_map) : ptr(_ptr), this_map(_this_map) {
                // check if the contents of this_map->iter_vec are equal to the current contents of this_map->table, see at() method
                for (typename My::Vector<std::pair<T1, T2>>::iterator it = this_map->iter_vec.begin(); it != this_map->iter_vec.end(); it++) {
                    if (it->second != this_map->at(it->first)) {
                        it->second = this_map->at(it->first);
                    }
                }
            }
            const std::pair<T1, T2>& operator* () { return *ptr; }
            const typename My::Vector<std::pair<T1, T2>>::iterator operator-> () { return ptr; }
            iterator& operator++ () { ++ptr; return *this; }
            iterator operator++ (int) { iterator tmp = *this; ++* this; return tmp; }
            iterator& operator-- () { --ptr; return *this; }
            iterator operator-- (int) { iterator tmp = *this; --* this; return tmp; }
            bool operator== (const iterator& it) { return ptr == it.ptr; }
            bool operator!= (const iterator& it) { return !(*this == it); }
        };

        iterator begin() { return iterator(iter_vec.begin(), this); }
        iterator end() { return iterator(iter_vec.end(), this); }
    };

    template <typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::HashMap(const Hash& _hash, const Allocator& _alloc) : hash(_hash), alloc(_alloc) {
        number_of_buckets = DEFAULT_NUMBER_OF_BUCKETS;
        buckets_used = 0;

        table = alloc.allocate(number_of_buckets);
        flag = state_alloc.allocate(number_of_buckets);
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::construct(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + i);
        }
    }

    template <typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::HashMap(int size, const Hash& _hash, const Allocator& _alloc) : hash(_hash), alloc(_alloc) {
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::HashMap(std::initializer_list<std::pair<T1, T2>> init_list, const Hash& _hash, const Allocator& _alloc) : HashMap<T1, T2, Hash, Allocator>::HashMap(init_list.size(), _hash, _alloc) {
        for (auto& el : init_list) {
            insert(el);
        }
    }

    template <typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::HashMap(const HashMap& other) {
        number_of_buckets = other.number_of_buckets;
        buckets_used = other.buckets_used;
        alloc = other.alloc;
        hash = other.hash;
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::HashMap(HashMap&& other) noexcept {
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

    template <typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>::~HashMap() {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, table + i);
            std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + i);
        }
        alloc.deallocate(table, number_of_buckets);
        state_alloc.deallocate(flag, number_of_buckets);
    }

    template <typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>& HashMap<T1, T2, Hash, Allocator>::operator = (const HashMap& other) {
        if (this != &other) {
            for (int i = 0; i < number_of_buckets; i++) {
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    HashMap<T1, T2, Hash, Allocator>& HashMap<T1, T2, Hash, Allocator>::operator = (HashMap&& other) noexcept {
        if (this != &other) {
            for (int i = 0; i < number_of_buckets; i++) {
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

    template <typename T1, typename T2, typename Hash, typename Allocator>
    T2& HashMap<T1, T2, Hash, Allocator>::operator [](const T1& key) { return at(key); }

    template <typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::rehash() {
        std::pair<T1, T2>* copy_of_table = std::move(table);
        BucketState* copy_of_flag = std::move(flag);
        size_t old_number_of_buckets = number_of_buckets;
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
                create_new_table(copy_of_table[i].first, copy_of_table[i].second);
        }

        for (std::size_t i = 0; i < old_number_of_buckets; i++) {
            std::allocator_traits<Allocator>::destroy(alloc, copy_of_table + i);
            std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, copy_of_flag + i);
        }
        alloc.deallocate(copy_of_table, old_number_of_buckets);
        state_alloc.deallocate(copy_of_flag, old_number_of_buckets);
    }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::create_new_table(const T1& key, const T2& value) {
        size_t index = hash(key) % number_of_buckets;
        while (true) {
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::destroy(alloc, table + index);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + index);
            }
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::construct(alloc, table + index, std::make_pair(key, value));
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::insert(const T1& key, const T2& value) {
        size_t index = hash(key) % number_of_buckets;
        while (true) {
            if (table[index].first == key && flag[index] == BucketState::PRESENT) {
                table[index].second = value;
                break;
            }
            if (flag[index] == BucketState::ABSENT) {
                std::allocator_traits<Allocator>::destroy(alloc, table + index);
                std::allocator_traits<std::allocator<BucketState>>::destroy(state_alloc, flag + index);
            }
            if (flag[index] == BucketState::ABSENT) {
                buckets_used++;
                std::allocator_traits<Allocator>::construct(alloc, table + index, std::make_pair(key, value));
                std::allocator_traits<std::allocator<BucketState>>::construct(state_alloc, flag + index, BucketState::PRESENT);
                iter_vec.push_back({ key, value });
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::insert(std::pair<T1, T2> pair_key_value) { insert(pair_key_value.first, pair_key_value.second); }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::erase(const T1& key) {
        size_t index = hash(key) % number_of_buckets;
        if (table[index].first != key) {
            do {
                index++;
                if (index >= number_of_buckets) {
                    index = 0;
                }
            } while (table[index].first != key && flag[index] != BucketState::ABSENT);
        }
        if (table[index].first == key && flag[index] != BucketState::DELETED) {
            flag[index] = BucketState::DELETED;
            iter_vec.erase(std::remove_if(iter_vec.begin(), iter_vec.end(), [key](std::pair<T1, T2> i) { return i.first == key; }));
        }
    }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    T2& HashMap<T1, T2, Hash, Allocator>::at(const T1& key) {
        size_t index = hash(key) % number_of_buckets;
        if (table[index].first != key || flag[index] != BucketState::PRESENT) {
            do {
                index++;
                if (index >= number_of_buckets) {
                    index = 0;
                }
            } while ((table[index].first != key || flag[index] != BucketState::PRESENT) && flag[index] != BucketState::ABSENT);
        }
        if (table[index].first == key && flag[index] == BucketState::PRESENT) {
            return table[index].second; // if we change the value this way, then the contents of this->iter_vec will be different from the contents of this->table
        }
        insert(key, T2());  // if we insert a new element this way, then the contents of this->iter_vec will be different from the contents of this->table
        return table[bucket(key)].second; // so when creating a new iterator we have to make them the same
    }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::clear() {
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

    template<typename T1, typename T2, typename Hash, typename Allocator>
    std::size_t HashMap<T1, T2, Hash, Allocator>::size() const noexcept { return iter_vec.size(); }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    std::size_t HashMap<T1, T2, Hash, Allocator>::bucket_count() const noexcept { return number_of_buckets; }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    bool HashMap<T1, T2, Hash, Allocator>::empty() const noexcept { return size() == 0; }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    int HashMap<T1, T2, Hash, Allocator>::bucket(const T1& key) const noexcept {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            if (table[i].first == key && flag[i] == BucketState::PRESENT) return i;
        }
        return -1;
    }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    bool HashMap<T1, T2, Hash, Allocator>::count(const T1& key) const noexcept {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            if (table[i].first == key && flag[i] == BucketState::PRESENT) return true;
        }
        return false;
    }

    template<typename T1, typename T2, typename Hash, typename Allocator>
    void HashMap<T1, T2, Hash, Allocator>::display() const {
        for (std::size_t i = 0; i < number_of_buckets; i++) {
            std::cout << i << ": key: " << table[i].first << "; value: " << table[i].second << "; flag: " << static_cast<int>(flag[i]) << std::endl;
        }
    }
}

int main() {
    std::cout << "My::HashMap<std::string, int> A\n";

    My::HashMap <std::string, int> A(4);

    A.insert({ "Apple", 150 });
    A.insert("Banana", 1000);
    A.insert("Orange", 110);
    A.insert("Banana", 500);
    A.insert(std::make_pair("Carrot", 250));
    A.erase("Orange");
    A["Potato"] = 450;

    for (auto& i : A)
    {
        std::cout << i.first << " " << i.second << "\n";
    }

    std::cout << "A[\"Apple\"]: " << A["Apple"] << "\nA.bucket(\"Apple\"): " << A.bucket("Apple") << "\nA.size(): " << A.size() << "\nA.count(\"Orange\"): " << A.count("Orange") << "\n\n";;

    std::cout << "B = A\n";

    My::HashMap<std::string, int> B = A;

    for (auto& i : B)
    {
        std::cout << i.first << " " << i.second << "\n";
    }
    std::cout << "\n";

    std::cout << "initializer_list\n";

    My::HashMap <std::string, std::string> C{ { "AAA", "aaa" }, { "BBB", "bbb" }, { "CCC", "ccc" } };

    C["DDD"] = "ddd";

    for (auto& i : C)
    {
        std::cout << i.first << " " << i.second << "\n";
    }
    std::cout << "\n";

    std::cout << "custom hash function + custom allocator\n";
    My::HashMap <int, int, Test::Hash<int>, Test::Allocator<std::pair<int, int>>> D;

    D.insert(1, 100);
    D.insert(2, 200);
    D.erase(2);

    D.display();

    return 0;
}