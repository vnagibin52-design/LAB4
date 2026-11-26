#pragma once

#include <algorithm>
#include <memory>
#include <new>
#include <unordered_map>
#include <utility>
#include <vector>

struct Pool {
    static constexpr std::size_t chunk_size = 4096;

    struct Header {
        std::size_t size;
        Header*     next;
    };

    std::vector<std::unique_ptr<unsigned char[]>> chunks;
    unsigned char* cur  = nullptr;
    std::size_t left = 0;

    std::size_t max_block_count = 10;
    std::size_t block_count = 0;

    std::unordered_map<std::size_t, Header*> free_lists;

    static unsigned char* align_up(unsigned char* p, std::size_t a) {
        auto u = reinterpret_cast<std::uintptr_t>(p);
        return reinterpret_cast<unsigned char*>((u + a - 1) & ~static_cast<std::uintptr_t>(a - 1));
    }

    unsigned char* bump(std::size_t bytes, std::size_t align) {
        unsigned char* h = align_up(cur, alignof(Header));
        unsigned char* data = align_up(h + sizeof(Header), align);
        std::size_t need = static_cast<std::size_t>(data - cur) + bytes;

        if (need > left) {
            std::size_t sz = std::max(chunk_size, need);
            chunks.emplace_back(new unsigned char[sz]);
            cur  = chunks.back().get();
            left = sz;
            return bump(bytes, align);
        }

        reinterpret_cast<Header*>(h)->size = bytes;
        cur  = data + bytes;
        left -= need;
        return data;
    }

    unsigned char* give(std::size_t bytes, std::size_t align) {
        auto it = free_lists.find(bytes);
        if (it != free_lists.end() && it->second) {
            Header* h = it->second;
            it->second = h->next;
            return reinterpret_cast<unsigned char*>(h) + sizeof(Header);
        }
        if (max_block_count && block_count >= max_block_count)
            throw std::bad_alloc();
        ++block_count;
        return bump(bytes, align);
    }

    void back(unsigned char* p, std::size_t bytes) {
        Header* h = reinterpret_cast<Header*>(p - sizeof(Header));
        h->next = free_lists[bytes];
        free_lists[bytes] = h;
    }

    void release_all(bool keep_chunks = true) {
        free_lists.clear();
        cur = nullptr;
        left = 0;
        if (!keep_chunks) chunks.clear();
    }
};

template <typename T>
struct MyAllocator {
    using value_type = T;

    std::shared_ptr<Pool> pool;

    MyAllocator() : pool(std::make_shared<Pool>()) {}

    template <typename U>
    MyAllocator(const MyAllocator<U>& other) noexcept : pool(other.pool) {}

    T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(pool->give(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        if (p) pool->back(reinterpret_cast<unsigned char*>(p), n * sizeof(T));
    }

    template <typename U>
    bool operator==(const MyAllocator<U>& o) const noexcept { return pool == o.pool; }
    template <typename U>
    bool operator!=(const MyAllocator<U>& o) const noexcept { return !(*this == o); }
};