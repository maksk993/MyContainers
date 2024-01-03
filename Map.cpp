#include <iostream>
#include <utility>
#include <initializer_list>
#include <stack>

namespace My {
    template <typename T1, typename T2>
    class Map {
        enum class Color { BLACK, RED };

        struct TreeNode {
            TreeNode(std::pair<T1, T2> _val, Color _color, TreeNode* _parent = nullptr) :
                val(_val), color(_color), left(nullptr), right(nullptr), parent(_parent) {}

            std::pair<T1, T2> val;
            Color color;
            TreeNode* left;
            TreeNode* right;
            TreeNode* parent;
        };

        TreeNode* root;
        TreeNode* max_node;
        std::size_t sz;

        void clear_traverse(TreeNode* cur);
        void copy_traverse(TreeNode* cur, TreeNode* other_cur);
        TreeNode* get_max_node(TreeNode* cur) const;
        void balancing_after_insert(TreeNode* cur);
        void right_rotation(TreeNode* y, TreeNode* g);
        void left_rotation(TreeNode* y, TreeNode* g);

    public:
        class iterator {
            TreeNode* ptr;
            Map<T1, T2>* this_map;
            std::stack<TreeNode*> used;
        public:
            iterator() = default;
            iterator(TreeNode* _ptr, Map<T1, T2>* _this_map) : ptr(_ptr), this_map(_this_map) {}
            bool operator ==(const iterator& other) { return ptr == other.ptr; }
            bool operator !=(const iterator& other) { return !(*this == other); }
            iterator& operator++() {
                if (ptr == this_map->max_node) ptr = ptr->right;

                else if (ptr->left && !used.empty() && used.top() != ptr->left) {
                    used.pop();
                    used.push(ptr);
                    ptr = ptr->left;
                }
                else if (ptr->right && !used.empty() && used.top() != ptr->right) {
                    used.pop();
                    used.push(ptr);
                    ptr = ptr->right;

                    while (ptr->left) {
                        ptr = ptr->left;
                    }
                }
                else if (ptr->parent) {
                    if (!used.empty()) used.pop();
                    used.push(ptr);
                    ptr = ptr->parent;

                    while (!used.empty() && used.top() == ptr->right) {
                        used.pop();
                        used.push(ptr);
                        ptr = ptr->parent;
                    }
                }

                return *this;
            }
            iterator operator++(int) {
                iterator tmp(ptr);
                ++* this;
                return tmp;
            }
            const std::pair<T1, T2>& operator*() { return ptr->val; }
        };

        Map();
        Map(std::initializer_list<std::pair<T1, T2>> init_list);
        Map(const Map& other);
        Map(Map&& other) noexcept;

        ~Map();

        Map& operator=(const Map& other);
        Map& operator=(Map&& other) noexcept;
        T2& operator[](T1 key);

        void insert(const T1& key, const T2& value);
        void insert(std::pair<T1, T2> value);
        T2& at(const T1& key);
        void clear();
        bool empty() const noexcept;
        std::size_t size() const noexcept;
        bool count(const T1& key) const noexcept;

        iterator begin();
        iterator end();
    };

    template<typename T1, typename T2>
    void Map<T1, T2>::clear_traverse(TreeNode* cur) {
        if (cur->left) clear_traverse(cur->left);
        if (cur->right) clear_traverse(cur->right);
        delete cur;
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::copy_traverse(TreeNode* cur, TreeNode* other_cur) {
        if (other_cur->left) {
            cur->left = new TreeNode(other_cur->left->val, other_cur->left->color, cur);
            copy_traverse(cur->left, other_cur->left);
        }

        if (other_cur->right) {
            cur->right = new TreeNode(other_cur->right->val, other_cur->right->color, cur);
            copy_traverse(cur->right, other_cur->right);
        }
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::balancing_after_insert(TreeNode* cur) {
        if (cur == root) {
            cur->color = Color::BLACK;
            return;
        }

        TreeNode* pParent = cur->parent;
        TreeNode* pGrandparent = nullptr;
        if (pParent->parent) pGrandparent = pParent->parent;
        if (!pGrandparent) return;

        TreeNode* pUncle = nullptr;
        if (pGrandparent->val.first > pParent->val.first) {
            if (pGrandparent->right) pUncle = pGrandparent->right;
        }
        else {
            if (pGrandparent->left) pUncle = pGrandparent->left;
        }

        if (!pUncle || pUncle->color == Color::BLACK) {
            if ((cur == pParent->left && pParent == pGrandparent->left) || (cur == pParent->right && pParent == pGrandparent->right)) {
                if (cur == pParent->left && pParent == pGrandparent->left) {
                    right_rotation(pParent, pGrandparent);
                }
                else {
                    left_rotation(pParent, pGrandparent);
                }
                pGrandparent->color = Color::RED;
                pParent->color = Color::BLACK;
            }
            else {
                if (cur == pParent->right && pParent == pGrandparent->left) {
                    left_rotation(cur, pParent);
                    right_rotation(cur, pGrandparent);
                }
                else {
                    right_rotation(cur, pParent);
                    left_rotation(cur, pGrandparent);
                }
                pGrandparent->color = Color::RED;
                cur->color = Color::BLACK;
            }
        }
        else {
            pUncle->color = Color::BLACK;
            pParent->color = Color::BLACK;
            pGrandparent->color = Color::RED;
            if (pGrandparent == root || pGrandparent->parent->color == Color::RED) balancing_after_insert(pGrandparent);
        }
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::right_rotation(TreeNode* pChild, TreeNode* pParent) {
        pChild->parent = pParent->parent;

        if (pParent != root) {
            if (pParent->parent->left == pParent) pParent->parent->left = pChild;
            else if (pParent->parent->right == pParent) pParent->parent->right = pChild;
        }
        pParent->parent = pChild;

        pParent->left = pChild->right;
        if (pChild->right) pChild->right->parent = pParent;
        pChild->right = pParent;

        if (pParent == root) root = pChild;
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::left_rotation(TreeNode* pChild, TreeNode* pParent) {
        pChild->parent = pParent->parent;

        if (pParent != root) {
            if (pParent->parent->left == pParent) pParent->parent->left = pChild;
            else if (pParent->parent->right == pParent) pParent->parent->right = pChild;
        }
        pParent->parent = pChild;

        pParent->right = pChild->left;
        if (pChild->left) pChild->left->parent = pParent;
        pChild->left = pParent;

        if (pParent == root) root = pChild;
    }

    template<typename T1, typename T2>
    typename Map<T1, T2>::TreeNode* Map<T1, T2>::get_max_node(TreeNode* cur) const {
        while (cur->right) cur = cur->right;
        return cur;
    }

    template<typename T1, typename T2>
    Map<T1, T2>::Map() : sz(0), root(nullptr), max_node(nullptr) {}

    template<typename T1, typename T2>
    Map<T1, T2>::Map(std::initializer_list<std::pair<T1, T2>> init_list) : Map() {
        for (auto& el : init_list) {
            insert(el);
        }
    }

    template<typename T1, typename T2>
    Map<T1, T2>::Map(const Map& other) : sz(other.sz) {
        if (other.root) {
            root = new TreeNode(other.root->val, other.root->color);
            copy_traverse(root, other.root);
            max_node = get_max_node(root);
        }
        else root = max_node = nullptr;
    }

    template<typename T1, typename T2>
    Map<T1, T2>::Map(Map&& other) noexcept : sz(other.sz), root(other.root), max_node(other.max_node) {
        other.root = nullptr;
        other.max_node = nullptr;
        other.sz = 0;
    }

    template<typename T1, typename T2>
    Map<T1, T2>::~Map() { clear(); }

    template<typename T1, typename T2>
    Map<T1, T2>& Map<T1, T2>::operator=(const Map& other) {
        if (this != &other) {
            clear();

            sz = other.sz;
            if (other.root) {
                root = new TreeNode(other.root->val, other.root->color);
                copy_traverse(root, other.root);
                max_node = get_max_node(root);
            }
            else root = max_node = nullptr;
        }
        return *this;
    }

    template<typename T1, typename T2>
    Map<T1, T2>& Map<T1, T2>::operator=(Map&& other) noexcept {
        if (this != &other) {
            sz = other.sz;
            root = other.root;
            max_node = other.max_node;

            other.sz = 0;
            other.root = nullptr;
        }
        return *this;
    }

    template<typename T1, typename T2>
    T2& Map<T1, T2>::operator[](T1 key) { return at(key); }

    template<typename T1, typename T2>
    void Map<T1, T2>::insert(const T1& key, const T2& value) {
        if (!root) {
            root = new TreeNode({ key, value }, Color::BLACK);
            max_node = root;
            sz++;
            return;
        }

        bool can_be_max = true;
        TreeNode* cur = root;
        while (true) {
            if (key > cur->val.first) {
                if (cur->right) cur = cur->right;
                else {
                    cur->right = new TreeNode({ key, value }, Color::RED, cur);
                    if (can_be_max) max_node = cur->right;
                    if (cur->color == Color::RED) balancing_after_insert(cur->right);
                    break;
                }
            }
            else if (key < cur->val.first) {
                can_be_max = false;
                if (cur->left) cur = cur->left;
                else {
                    cur->left = new TreeNode({ key, value }, Color::RED, cur);
                    if (cur->color == Color::RED) balancing_after_insert(cur->left);
                    break;
                }
            }
            else {
                cur->val.second = value;
                return;
            }
        }
        sz++;
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::insert(std::pair<T1, T2> value) {
        insert(value.first, value.second);
    }

    template<typename T1, typename T2>
    T2& Map<T1, T2>::at(const T1& key) {
        if (!root) {
            root = new TreeNode({ key, T2() }, Color::BLACK);
            sz++;
            return root->val.second;
        }

        bool can_be_max = true;
        TreeNode* cur = root;
        while (true) {
            if (key > cur->val.first) {
                if (cur->right) cur = cur->right;
                else {
                    cur->right = new TreeNode({ key, T2() }, Color::RED, cur);
                    if (can_be_max) max_node = cur->right;
                    sz++;
                    return cur->right->val.second;
                }
            }
            else if (key < cur->val.first) {
                can_be_max = false;
                if (cur->left) cur = cur->left;
                else {
                    cur->left = new TreeNode({ key, T2() }, Color::RED, cur);
                    sz++;
                    return cur->left->val.second;
                }
            }
            else return cur->val.second;
        }
    }

    template<typename T1, typename T2>
    void Map<T1, T2>::clear() {
        if (!root) return;

        clear_traverse(root);

        sz = 0;
        root = nullptr;
    }

    template<typename T1, typename T2>
    bool Map<T1, T2>::empty() const noexcept { return sz == 0; }

    template<typename T1, typename T2>
    std::size_t Map<T1, T2>::size() const noexcept { return sz; }

    template<typename T1, typename T2>
    bool Map<T1, T2>::count(const T1& key) const noexcept {
        if (!root) return false;

        TreeNode* cur = root;
        while (true) {
            if (key > cur->val.first) {
                if (cur->right) cur = cur->right;
                else return false;
            }
            else if (key < cur->val.first) {
                if (cur->left) cur = cur->left;
                else return false;
            }
            else return true;
        }
    }

    template<typename T1, typename T2>
    typename Map<T1, T2>::iterator Map<T1, T2>::begin() {
        if (!root) return iterator(nullptr, this);

        TreeNode* cur = root;
        while (cur->left) {
            cur = cur->left;
        }
        return iterator(cur, this);
    }

    template<typename T1, typename T2>
    typename Map<T1, T2>::iterator Map<T1, T2>::end() {
        if (!root) return iterator(nullptr, this);

        return iterator(max_node->right, this);
    }
}

int main() {
    My::Map<int, int> m{ {200,7}, {150,5}, {250,9}, {120,4}, {160,6}, {230,8}, {280,11}, {270,10}, {90,1}, {110, 3}, {100,2} };

    for (auto& i : m) {
        std::cout << i.first << " " << i.second << "\n";
    }
    std::cout << "\n";

    std::cout << "m.size(): " << m.size() << "\n";
    std::cout << "m[150]: " << m[150] << "\n\n";

    m[300] = 12;

    for (auto& i : m) {
        std::cout << i.first << " " << i.second << "\n";
    }

    return 0;
}