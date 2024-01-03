#include <iostream>
#include <utility>
#include <initializer_list>
#include <stack>

namespace My {
    template <typename T>
    class Set {
        enum class Color { BLACK, RED };

        struct TreeNode {
            TreeNode(T _val, Color _color, TreeNode* _parent = nullptr) :
                val(_val), color(_color), left(nullptr), right(nullptr), parent(_parent) {}

            T val;
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
            Set<T>* this_set;
            std::stack<TreeNode*> used;
        public:
            iterator() = default;
            iterator(TreeNode* _ptr, Set<T>* _this_set) : ptr(_ptr), this_set(_this_set) {}
            bool operator ==(const iterator& other) { return ptr == other.ptr; }
            bool operator !=(const iterator& other) { return !(*this == other); }
            iterator& operator++() {
                if (ptr == this_set->max_node) ptr = ptr->right;

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
            const T& operator*() { return ptr->val; }
        };

        Set();
        Set(std::initializer_list<T> init_list);
        Set(const Set& other);
        Set(Set&& other) noexcept;

        ~Set();

        Set& operator=(const Set& other);
        Set& operator=(Set&& other) noexcept;

        void insert(const T& key);
        void clear();
        bool empty() const noexcept;
        std::size_t size() const noexcept;
        bool count(const T& key) const noexcept;

        iterator begin();
        iterator end();
    };

    template<typename T>
    void Set<T>::clear_traverse(TreeNode* cur) {
        if (cur->left) clear_traverse(cur->left);
        if (cur->right) clear_traverse(cur->right);
        delete cur;
    }

    template<typename T>
    void Set<T>::copy_traverse(TreeNode* cur, TreeNode* other_cur) {
        if (other_cur->left) {
            cur->left = new TreeNode(other_cur->left->val, other_cur->left->color, cur);
            copy_traverse(cur->left, other_cur->left);
        }

        if (other_cur->right) {
            cur->right = new TreeNode(other_cur->right->val, other_cur->right->color, cur);
            copy_traverse(cur->right, other_cur->right);
        }
    }

    template<typename T>
    void Set<T>::balancing_after_insert(TreeNode* cur) {
        if (cur == root) {
            cur->color = Color::BLACK;
            return;
        }

        TreeNode* pParent = cur->parent;
        TreeNode* pGrandparent = nullptr;
        if (pParent->parent) pGrandparent = pParent->parent;
        if (!pGrandparent) return;

        TreeNode* pUncle = nullptr;
        if (pGrandparent->val > pParent->val) {
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

    template<typename T>
    void Set<T>::right_rotation(TreeNode* pChild, TreeNode* pParent) {
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

    template<typename T>
    void Set<T>::left_rotation(TreeNode* pChild, TreeNode* pParent) {
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

    template<typename T>
    typename Set<T>::TreeNode* Set<T>::get_max_node(TreeNode* cur) const {
        while (cur->right) cur = cur->right;
        return cur;
    }

    template<typename T>
    Set<T>::Set() : sz(0), root(nullptr), max_node(nullptr) {}

    template<typename T>
    Set<T>::Set(std::initializer_list<T> init_list) : Set() {
        for (auto& el : init_list) {
            insert(el);
        }
    }

    template<typename T>
    Set<T>::Set(const Set& other) : sz(other.sz) {
        if (other.root) {
            root = new TreeNode(other.root->val, other.root->color);
            copy_traverse(root, other.root);
            max_node = get_max_node(root);
        }
        else root = max_node = nullptr;
    }

    template<typename T>
    Set<T>::Set(Set&& other) noexcept : sz(other.sz), root(other.root), max_node(other.max_node) {
        other.root = nullptr;
        other.max_node = nullptr;
        other.sz = 0;
    }

    template<typename T>
    Set<T>::~Set() { clear(); }

    template<typename T>
    Set<T>& Set<T>::operator=(const Set& other) {
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

    template<typename T>
    Set<T>& Set<T>::operator=(Set&& other) noexcept {
        if (this != &other) {
            sz = other.sz;
            root = other.root;
            max_node = other.max_node;

            other.sz = 0;
            other.root = nullptr;
        }
        return *this;
    }

    template<typename T>
    void Set<T>::insert(const T& key) {
        if (!root) {
            root = new TreeNode({ key }, Color::BLACK);
            max_node = root;
            sz++;
            return;
        }

        bool can_be_max = true;
        TreeNode* cur = root;
        while (true) {
            if (key > cur->val) {
                if (cur->right) cur = cur->right;
                else {
                    cur->right = new TreeNode({ key }, Color::RED, cur);
                    if (can_be_max) max_node = cur->right;
                    if (cur->color == Color::RED) balancing_after_insert(cur->right);
                    break;
                }
            }
            else if (key < cur->val) {
                can_be_max = false;
                if (cur->left) cur = cur->left;
                else {
                    cur->left = new TreeNode({ key }, Color::RED, cur);
                    if (cur->color == Color::RED) balancing_after_insert(cur->left);
                    break;
                }
            }
            else return;
        }
        sz++;
    }

    template<typename T>
    void Set<T>::clear() {
        if (!root) return;

        clear_traverse(root);

        sz = 0;
        root = nullptr;
    }

    template<typename T>
    bool Set<T>::empty() const noexcept { return sz == 0; }

    template<typename T>
    std::size_t Set<T>::size() const noexcept { return sz; }

    template<typename T>
    bool Set<T>::count(const T& key) const noexcept {
        if (!root) return false;

        TreeNode* cur = root;
        while (true) {
            if (key > cur->val) {
                if (cur->right) cur = cur->right;
                else return false;
            }
            else if (key < cur->val) {
                if (cur->left) cur = cur->left;
                else return false;
            }
            else return true;
        }
    }

    template<typename T>
    typename Set<T>::iterator Set<T>::begin() {
        if (!root) return iterator(nullptr, this);

        TreeNode* cur = root;
        while (cur->left) {
            cur = cur->left;
        }
        return iterator(cur, this);
    }

    template<typename T>
    typename Set<T>::iterator Set<T>::end() {
        if (!root) return iterator(nullptr, this);

        return iterator(max_node->right, this);
    }
}

int main() {
    My::Set<int> s{ 1,2,3,4,5,6,6,3,1,1 };
    s.insert(11);

    for (auto& i : s)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";
    
    My::Set<int> t = s;
    for (auto& i : t)
    {
        std::cout << i << " ";
    }
    std::cout << "\n";

    t.clear();

    std::cout << "s.size(): " << s.size() << " t.size(): " << t.size() << "\n";

    return 0;
}