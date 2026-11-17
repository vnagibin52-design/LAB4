#include <vector>
#include <map>
#include <iostream>
#include "MyAllocator.h"

int main() {

    MyAllocator<int> alloc;

    alloc.allocate(10);

    std::vector<int, MyAllocator<int>> v1(alloc);

    for (int i = 0; i < 20; i++){
        v1.push_back(i);
    }

    for (auto value : v1){
        std::cout << value << "   " << v1.capacity() << "\n";
    }

    std::cout << v1.capacity() << "\n";

    /*

    MyAllocator<std::pair<const int, int>> alloc;

    std::map<int, int, std::less<int>, MyAllocator<std::pair<const int, int>>> MyMap(alloc);

    for (int i = 0; i < 10; i++){
        MyMap.emplace(i, "tuts");
    }

    for (auto value : MyMap){
        std::cout << value.first << " " << value.second << "\n";
    }
        */
}
