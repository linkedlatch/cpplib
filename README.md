# cpplib

A header-only C++ container library.

## Requirements

C++11 or later — current containers only use C++11 features (move semantics,
variadic templates, `initializer_list`, etc.).

## Usage

```cpp
#include <vector.hpp>

cpplib::Vector<int> v = {1, 2, 3};
v.push_back(4);
```

## Containers

- `cpplib::Vector<T>` — dynamic array similar to `std::vector`

More containers will be added over time.
