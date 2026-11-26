#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include "Storable.h"

template <Storable T, typename Allocator = std::allocator<T>>
class OneWaySContainer {
private:
    struct Node {
        T value;
        Node* next;

        Node(T v) : value(v), next(nullptr) {}
    };

    using NodeAlloc =
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    Node* head = nullptr;
    Node* tail = nullptr;
    std::size_t size_ = 0;
    NodeAlloc alloc_;

    Node* make_node(T value) {
        Node* node = alloc_.allocate(1);
        try {
            std::allocator_traits<NodeAlloc>::construct(alloc_, &node->value, std::move(value));
        } catch (...) {
            alloc_.deallocate(node, 1);
            throw;
        }
        node->next = nullptr;
        return node;
    }

    void free_node(Node* node) noexcept {
        std::allocator_traits<NodeAlloc>::destroy(alloc_, &node->value);
        alloc_.deallocate(node, 1);
    }

public:
    OneWaySContainer() = default;

    explicit OneWaySContainer(const Allocator& alloc) : alloc_(alloc) {}

    OneWaySContainer(const OneWaySContainer& other)
        : head(nullptr), tail(nullptr), size_(0), alloc_(other.alloc_) {
        for (Node* cur = other.head; cur != nullptr; cur = cur->next)
            push_back(cur->value);
    }

    OneWaySContainer(OneWaySContainer&& other) noexcept
        : head(other.head), tail(other.tail), size_(other.size_), alloc_(std::move(other.alloc_)) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size_ = 0;
    }

    ~OneWaySContainer() {
        clear();
    }

    void clear() {
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            free_node(temp);
        }
        tail = nullptr;
        size_ = 0;
    }

    OneWaySContainer& operator=(const OneWaySContainer& other) {
        if (this == &other) return *this;
        OneWaySContainer temp(other);
        swap(temp);
        return *this;
    }

    void swap(OneWaySContainer& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(size_, other.size_);
        std::swap(alloc_, other.alloc_);
    }

    OneWaySContainer& operator=(OneWaySContainer&& other) noexcept {
        if (this == &other) return *this;

        clear();

        head = other.head;
        tail = other.tail;
        size_ = other.size_;
        alloc_ = std::move(other.alloc_);

        other.head = nullptr;
        other.tail = nullptr;
        other.size_ = 0;

        return *this;
    }

    std::size_t size() const {
        return size_;
    }

    void push_back(T value) {
        Node* newNode = make_node(std::move(value));
        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++size_;
    }

    void push_front(T value) {
        Node* newNode = make_node(std::move(value));
        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        } else {
            newNode->next = head;
            head = newNode;
        }
        ++size_;
    }

    void pop_back() {
        Node* nodeForDelete = tail;
        if (head == tail) {
            head = nullptr;
            tail = nullptr;
        } else {
            Node* current = head;
            while (current->next != tail) {
                current = current->next;
            }
            tail = current;
            tail->next = nullptr;
        }
        free_node(nodeForDelete);
        --size_;
    }

    void pop_front() {
        Node* nodeForDelete = head;
        if (head == tail) {
            head = nullptr;
            tail = nullptr;
        } else {
            head = head->next;
        }
        free_node(nodeForDelete);
        --size_;
    }

    void insert(Node* pos, T value) {
        Node* newNode = make_node(std::move(value));
        newNode->next = pos;

        if (pos == head) {
            head = newNode;
        } else {
            Node* current = head;
            while (current->next != pos) {
                current = current->next;
            }
            current->next = newNode;
        }
        ++size_;
    }

    void erase(Node* pos) {
        if (pos == head) {
            head = pos->next;
        } else {
            Node* current = head;
            while (current->next != pos) {
                current = current->next;
            }
            current->next = pos->next;
            if (pos == tail)
                tail = current;
        }
        free_node(pos);
        --size_;
    }

    class iterator {
        Node* node;
    public:
        explicit iterator(Node* n) : node(n) {}

        T& operator*() {
            if (node == nullptr)
                throw std::out_of_range("Итератор разыменование end()");
            return node->value;
        }

        iterator& operator++() {
            if (node == nullptr)
                throw std::out_of_range("Итератор за пределами списка");
            node = node->next;
            return *this;
        }

        bool operator!=(const iterator& other) {
            return node != other.node;
        }

        Node* get_node() {
            return node;
        }
    };

    class const_iterator {
        const Node* node;
    public:
        explicit const_iterator(Node* n) : node(n) {}

        const T& operator*() const {
            if (node == nullptr)
                throw std::out_of_range("const Итератор разыменование end()");
            return node->value;
        }

        const_iterator& operator++() {
            if (node == nullptr)
                throw std::out_of_range("const Итератор за пределами списка");
            node = node->next;
            return *this;
        }

        bool operator!=(const const_iterator& other) {
            return node != other.node;
        }

        Node* get_node() {
            return node;
        }
    };

    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }

    const_iterator begin() const { return const_iterator(head); }
    const_iterator end()   const { return const_iterator(nullptr); }
};