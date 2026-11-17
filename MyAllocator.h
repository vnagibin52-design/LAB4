#pragma once

#include <cstddef>
#include <memory>
#include <new>

template <typename T>
struct MyAllocator {
    using value_type = T;

    struct Pool {
        std::vector<std::byte> buf;
        std::size_t used = 0;

        std::byte* give(std::size_t bytes) {
            if (used + bytes > buf.size())
                buf.resize(used + bytes);
            std::byte* p = buf.data() + used;
            used += bytes;
            return p;
        }
    };

    std::shared_ptr<Pool> pool;

    MyAllocator() : pool(std::make_shared<Pool>()) {}

    template <typename U>
    MyAllocator(const MyAllocator<U>& o) noexcept : pool(o.pool) {}

    template <typename U>
    struct rebind { using other = MyAllocator<U>; };

    T* allocate(std::size_t n) {
        return reinterpret_cast<T*>(pool->give(n * sizeof(T)));
    }

    void deallocate(T*, std::size_t) noexcept {}

    void reserve(std::size_t n) { pool->buf.resize(n * sizeof(T)); }

    template <typename U>
    bool operator==(const MyAllocator<U>& o) const noexcept { return pool == o.pool; }
    template <typename U>
    bool operator!=(const MyAllocator<U>& o) const noexcept { return !(*this == o); }
};