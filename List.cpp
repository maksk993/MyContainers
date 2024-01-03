#include <iostream>
#include <utility>
#include <stdexcept>
#include <initializer_list>
#include <memory>

namespace My {
	template <typename T>
	class List {
		struct Node {
			Node() : val(T()), next(nullptr), prev(nullptr) {}
			Node(T _val) : val(_val), next(nullptr), prev(nullptr) {}
			T val;
			Node* next;
			Node* prev;
		};
		Node* tail;
		Node* head;
		std::size_t sz;

	public:
		class iterator {
			Node* ptr;
		public:
			friend class List;
			iterator() = default;
			iterator(Node* _ptr) : ptr(_ptr) {}
			iterator& operator++() { ptr = ptr->next; return *this; }
			iterator operator++(int) { iterator tmp = *this;  ptr = ptr->next; return tmp; }
			bool operator==(const iterator& other) const noexcept { return ptr == other.ptr; }
			bool operator!=(const iterator& other) const noexcept { return !(*this == other); }
			T& operator*() const noexcept { return ptr->val; }
			T* operator->() const noexcept { return ptr; }
		};

		List();
		List(std::size_t size);
		List(std::size_t size, const T& value);
		List(std::initializer_list<T> init_list);
		List(const List& other);
		List(List&& other) noexcept;

		~List();

		List& operator= (const List& other);
		List& operator= (List&& other) noexcept;

		iterator insert(iterator position, const T& element);
		iterator insert(iterator position, int number, const T& element);
		iterator insert(iterator position, std::initializer_list<T> init_list);
		iterator erase(iterator first);
		iterator erase(iterator first, iterator second);
		void push_back(const T& element);
		void push_front(const T& element);
		void pop_back();
		void pop_front();
		void resize(int size);
		void clear();
		T& front() const noexcept { return head->val; }
		T& back() const noexcept { return tail->val; }
		std::size_t size() const noexcept { return sz; }
		bool empty() const noexcept { return sz == 0; }

		iterator begin() { return iterator(head); };
		iterator end() { if (!sz) return iterator(tail); return iterator(tail->next); };
	};

	template<typename T>
	List<T>::List() : tail(nullptr), head(tail), sz(0) {}

	template<typename T>
	List<T>::List(std::size_t size) : sz(size) {
		if (size < 0) throw std::length_error("length error."); // EXCEPTION
		if (size == 0) head = tail = nullptr;
		else {
			head = new Node();
			tail = head;
			for (std::size_t i = 0; i < size - 1; i++) {
				tail->next = new Node();
				tail->next->prev = tail;
				tail = tail->next;
			}
		}
	}

	template<typename T>
	List<T>::List(std::size_t size, const T& value) : sz(size) {
		if (size < 0) throw std::length_error("length error."); // EXCEPTION
		if (size == 0) head = tail = nullptr;
		else {
			head = new Node(value);
			tail = head;
			for (std::size_t i = 0; i < size - 1; i++) {
				tail->next = new Node(value);
				tail->next->prev = tail;
				tail = tail->next;
			}
		}
	}

	template<typename T>
	List<T>::List(std::initializer_list<T> init_list) {
		for (auto& el : init_list) {
			push_back(el);
		}
	}

	template<typename T>
	List<T>::List(const List& other) : sz(other.sz) {
		if (sz == 0) { head = tail = nullptr; return; }

		head = new Node(other.head->val);
		tail = head;

		Node* cur = other.head->next;
		for (std::size_t i = 0; i < sz - 1; i++) {
			tail->next = new Node(cur->val);
			tail->next->prev = tail;
			tail = tail->next;
			cur = cur->next;
		}
	}

	template<typename T>
	List<T>::List(List&& other) noexcept : head(other.head), tail(other.tail), sz(other.sz) {
		other.sz = 0;
		other.head = nullptr;
		other.tail = nullptr;
	}

	template<typename T>
	List<T>::~List() { clear(); }

	template<typename T>
	List<T>& List<T>::operator=(const List& other) {
		if (this != &other) {
			while (head) {
				Node* forward = head->next;
				delete head;
				head = forward;
			}

			sz = other.sz;
			if (sz == 0) { head = tail = nullptr; return *this; }

			head = new Node(other.head->val);
			tail = head;

			Node* cur = other.head->next;
			for (std::size_t i = 0; i < sz - 1; i++) {
				tail->next = new Node(cur->val);
				tail->next->prev = tail;
				tail = tail->next;
				cur = cur->next;
			}
		}
		return *this;
	}

	template<typename T>
	List<T>& List<T>::operator=(List&& other) noexcept {
		if (this != &other) {
			while (head) {
				Node* forward = head->next;
				delete head;
				head = forward;
			}

			head = other.head;
			tail = other.tail;
			sz = other.sz;
			other.sz = 0;
			other.head = nullptr;
			other.tail = nullptr;
		}
		return *this;
	}

	template<typename T>
	typename List<T>::iterator List<T>::insert(iterator position, const T& element) { // FIXME
		if (position == begin()) {
			push_front(element);
			return iterator(head);
		}
		if (position == end()) {
			push_back(element);
			return iterator(tail);
		}

		position.ptr->prev->next = new Node(element);
		position.ptr->prev->next->prev = position.ptr->prev;
		position.ptr->prev = position.ptr->prev->next;
		position.ptr->prev->next = position.ptr; 
		sz++;

		return iterator(position.ptr->prev);
	}

	template<typename T>
	typename List<T>::iterator List<T>::insert(iterator position, int number, const T& element) {
		if (number < 0) throw std::length_error("length error."); // EXCEPTION
		iterator cur_pos = position;
		for (std::size_t i = number; i > 0; i--) {
			cur_pos = insert(cur_pos, element);
			++cur_pos;
		}
		if (!position.ptr) return iterator(tail); //
		return position;
	}

	template<typename T>
	typename List<T>::iterator List<T>::insert(iterator position, std::initializer_list<T> init_list) {
		iterator cur_pos = position;
		for (auto& el : init_list) {
			cur_pos = insert(cur_pos, el);
			++cur_pos;
		}
		if (!position.ptr) return iterator(tail); //
		return position;
	}

	template<typename T>
	typename List<T>::iterator List<T>::erase(iterator first) {
		if (sz == 0 || !first.ptr) throw std::out_of_range("list erase iterator outside range."); // EXCEPTION

		if (first == begin()) {
			pop_front();
			return iterator(head);
		}
		if (first == iterator(tail)) {
			pop_back();
			return iterator(tail);
		}

		iterator cur_pos(first.ptr->next);
		first.ptr->prev->next = first.ptr->next;
		first.ptr->next->prev = first.ptr->prev;

		sz--;
		delete first.ptr;
		return cur_pos;
	}

	template<typename T>
	typename List<T>::iterator List<T>::erase(iterator first, iterator second) {
		if (sz == 0 || !first.ptr) throw std::out_of_range("list erase iterator outside range."); // EXCEPTION
		if (first == second) return second;

		int offset = 0;
		for (iterator it = first; it != second; ++it) {
			offset++;
		}

		iterator cur_pos = first;
		for (std::size_t i = offset; i > 0; i--) {
			cur_pos = erase(cur_pos);
		}
		return cur_pos;
	}

	template<typename T>
	void List<T>::push_back(const T& element) {
		if (sz == 0) {
			head = new Node(element);
			tail = head;
		}
		else {
			tail->next = new Node(element);
			tail->next->prev = tail;
			tail = tail->next;
		}
		sz++;
	}

	template<typename T>
	void List<T>::push_front(const T& element) {
		if (sz == 0) {
			head = new Node(element);
			tail = head;
		}
		else {
			head->prev = new Node(element);
			head->prev->next = head;
			head = head->prev;
		}
		sz++;
	}

	template<typename T>
	void List<T>::pop_back() {
		if (!sz) throw std::out_of_range("pop_back called on empty list."); // EXCEPTION
		if (sz == 1) {
			delete tail;
			head = tail = nullptr;
		}
		else {
			tail = tail->prev;
			delete tail->next;
			tail->next = nullptr;
		}
		sz--;
	}

	template<typename T>
	void List<T>::pop_front() {
		if (!sz) throw std::out_of_range("pop_front called on empty list."); // EXCEPTION
		if (sz == 1) {
			delete tail;
			head = tail = nullptr;
		}
		else {
			head = head->next;
			delete head->prev;
			head->prev = nullptr;
		}
		sz--;
	}

	template<typename T>
	void List<T>::resize(int size) {
		if (size < 0) throw std::length_error("length error."); // EXCEPTION
		if (size > sz) {
			while (sz != size) {
				push_back(T());
			}
		}
		if (size < sz) {
			while (sz != size) {
				pop_back();
			}
		}
	}

	template<typename T>
	void List<T>::clear() {
		while (head) {
			Node* forward = head->next;
			delete head;
			head = forward;
		}
		head = tail = nullptr;
		sz = 0;
	}
}

int main() {
	My::List<int> a{ 1,2,3,4,5,6,7 };

	a.insert(a.begin(), 5, 10);
	a.erase(a.begin(), ++++a.begin());

	for (auto& i : a) {
		std::cout << i << " ";
	}
	std::cout << "\n";

	My::List<int> b = a;

	b.pop_back();
	b.pop_front();
	b.push_back(1000);
	b.push_front(0);
	
	for (auto& i : b) {
		std::cout << i << " ";
	}
	std::cout << "\n";

	std::cout << "a.size(): " << a.size() << " b.size(): " << b.size() << "\n";

	return 0;
}