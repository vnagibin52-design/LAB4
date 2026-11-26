#include <vector>
#include <map>
#include <iostream>
#include "MyAllocator.h"
#include "OneWaySContainer.h"

int main() {
    
    std::map<int, int> m1;

    for(int i = 0; i < 9; i++){
        int f = 1;
        for (int k = 2; k <= i; ++k) 
            f *= k;

        m1[i] = f;
    }

    for (const auto& [k, v] : m1)
        std::cout << k << " -> " << v << "\n";
    
    using MapAlloc = MyAllocator<std::pair<const int, int>>;

    MapAlloc alloc;
    std::map<int, int, std::less<>, MapAlloc> m2(alloc);

    for(int i = 0; i < 9; i++){
        int f = 1;
        for (int k = 2; k <= i; ++k) 
            f *= k;

        m2[i] = f;
    }

    for (const auto& [k, v] : m2)
        std::cout << k << " -> " << v << "\n";

    try {
        m2[10] = 1;
    } catch (const std::bad_alloc&) {
        std::cout << "аллокатор ограничен: 11-й элемент не выделяется\n";
    }

    OneWaySContainer<int> S1;

    for(int i = 0; i < 10; i++){
        int f = 1;
        for (int k = 2; k <= i; ++k) 
            f *= k;

        S1.push_back(f);
    }

    for (const auto& value : S1)
        std::cout << value << "\n";

    try {
        S1.push_back(1);
    } catch (const std::bad_alloc&) {
        std::cout << "аллокатор ограничен: 11-й элемент не выделяется\n";
    }

    OneWaySContainer<int, MapAlloc> S2;

    for(int i = 0; i < 10; i++){
        int f = 1;
        for (int k = 2; k <= i; ++k) 
            f *= k;

        S2.push_back(f);
    }

    for (const auto& value : S2)
        std::cout << value << "\n";

    try {
        S2.push_back(1);
    } catch (const std::bad_alloc&) {
        std::cout << "аллокатор ограничен: 11-й элемент не выделяется\n";
    }

}