#include <vector.hpp>

#include <iostream>

int main() {
  cpplib::Vector<int> v = {1, 2, 3};
  v.push_back(4);
  v.emplace_back(5);
  v.insert(0, 0);
  v.erase(1);

  for (int x : v) std::cout << x << ' ';
  std::cout << '\n';

  std::cout << "size=" << v.size() << " capacity=" << v.capacity() << '\n';
  std::cout << "front=" << v.front() << " back=" << v.back() << '\n';
}
