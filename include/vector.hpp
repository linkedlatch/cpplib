#pragma once

#include <cstddef>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <utility>

namespace cpplib {

template <typename T>
class Vector {
 public:
  using iterator = T*;
  using const_iterator = const T*;

  // Construction / assignment / destruction

  Vector() noexcept = default;

  explicit Vector(std::size_t count) {
    T* new_data = static_cast<T*>(::operator new(count * sizeof(T)));
    std::size_t constructed = 0;
    try {
      for (; constructed < count; ++constructed) {
        ::new (static_cast<void*>(new_data + constructed)) T();
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) new_data[i].~T();
      ::operator delete(new_data);
      throw;
    }
    data_ = new_data;
    size_ = count;
    capacity_ = count;
  }

  Vector(std::size_t count, const T& value) {
    T* new_data = static_cast<T*>(::operator new(count * sizeof(T)));
    std::size_t constructed = 0;
    try {
      for (; constructed < count; ++constructed) {
        ::new (static_cast<void*>(new_data + constructed)) T(value);
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) new_data[i].~T();
      ::operator delete(new_data);
      throw;
    }
    data_ = new_data;
    size_ = count;
    capacity_ = count;
  }

  Vector(std::initializer_list<T> init) {
    T* new_data = static_cast<T*>(::operator new(init.size() * sizeof(T)));
    std::size_t constructed = 0;
    try {
      for (const T& value : init) {
        ::new (static_cast<void*>(new_data + constructed)) T(value);
        ++constructed;
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) new_data[i].~T();
      ::operator delete(new_data);
      throw;
    }
    data_ = new_data;
    size_ = init.size();
    capacity_ = init.size();
  }

  Vector(const Vector& other) {
    T* new_data = static_cast<T*>(::operator new(other.size_ * sizeof(T)));
    std::size_t constructed = 0;
    try {
      for (; constructed < other.size_; ++constructed) {
        ::new (static_cast<void*>(new_data + constructed))
            T(other.data_[constructed]);
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) new_data[i].~T();
      ::operator delete(new_data);
      throw;
    }
    data_ = new_data;
    size_ = other.size_;
    capacity_ = other.size_;
  }

  Vector& operator=(const Vector& other) {
    if (this != &other) {
      Vector tmp(other);
      swap(tmp);
    }
    return *this;
  }

  Vector(Vector&& other) noexcept
      : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  Vector& operator=(Vector&& other) noexcept {
    if (this != &other) {
      destroy_all();
      ::operator delete(data_);
      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;
      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  ~Vector() {
    destroy_all();
    ::operator delete(data_);
  }

  // Element access

  T& operator[](std::size_t index) { return data_[index]; }
  const T& operator[](std::size_t index) const { return data_[index]; }

  T& at(std::size_t index) {
    if (index >= size_)
      throw std::out_of_range("Vector::at: index out of range");
    return data_[index];
  }

  const T& at(std::size_t index) const {
    if (index >= size_)
      throw std::out_of_range("Vector::at: index out of range");
    return data_[index];
  }

  T& front() { return data_[0]; }
  const T& front() const { return data_[0]; }

  T& back() { return data_[size_ - 1]; }
  const T& back() const { return data_[size_ - 1]; }

  T* data() noexcept { return data_; }
  const T* data() const noexcept { return data_; }

  // Iterators

  iterator begin() noexcept { return data_; }
  const_iterator begin() const noexcept { return data_; }
  iterator end() noexcept { return data_ + size_; }
  const_iterator end() const noexcept { return data_ + size_; }

  // Capacity

  bool empty() const { return size_ == 0; }
  std::size_t size() const { return size_; }
  std::size_t capacity() const { return capacity_; }
  std::size_t max_size() const noexcept { return std::size_t(-1) / sizeof(T); }

  void reserve(std::size_t new_capacity) {
    if (new_capacity > capacity_) reallocate(new_capacity);
  }

  void shrink_to_fit() {
    if (size_ < capacity_) reallocate(size_);
  }

  // Modifiers

  void resize(std::size_t new_size) {
    if (new_size < size_) {
      for (std::size_t i = new_size; i < size_; ++i) data_[i].~T();
    } else if (new_size > size_) {
      if (new_size > capacity_) reallocate(new_size);
      std::size_t constructed = size_;
      try {
        for (; constructed < new_size; ++constructed) {
          ::new (static_cast<void*>(data_ + constructed)) T();
        }
      } catch (...) {
        for (std::size_t i = size_; i < constructed; ++i) data_[i].~T();
        throw;
      }
    }
    size_ = new_size;
  }

  void assign(std::size_t count, const T& value) {
    destroy_all();
    size_ = 0;
    if (count > capacity_) reallocate(count);
    std::size_t constructed = 0;
    try {
      for (; constructed < count; ++constructed) {
        ::new (static_cast<void*>(data_ + constructed)) T(value);
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) data_[i].~T();
      throw;
    }
    size_ = count;
  }

  void push_back(const T& value) {
    if (size_ == capacity_) reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    ::new (static_cast<void*>(data_ + size_)) T(value);
    ++size_;
  }

  void push_back(T&& value) {
    if (size_ == capacity_) reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    ::new (static_cast<void*>(data_ + size_)) T(std::move(value));
    ++size_;
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (size_ == capacity_) reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    ::new (static_cast<void*>(data_ + size_)) T(std::forward<Args>(args)...);
    ++size_;
  }

  void insert(std::size_t index, const T& value) {
    if (index > size_)
      throw std::out_of_range("Vector::insert: index out of range");
    if (size_ == capacity_) reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
    if (index < size_) {
      ::new (static_cast<void*>(data_ + size_)) T(std::move(data_[size_ - 1]));
      for (std::size_t i = size_ - 1; i > index; --i) {
        data_[i] = std::move(data_[i - 1]);
      }
      data_[index] = value;
    } else {
      ::new (static_cast<void*>(data_ + index)) T(value);
    }
    ++size_;
  }

  void erase(std::size_t index) {
    if (index >= size_)
      throw std::out_of_range("Vector::erase: index out of range");
    for (std::size_t i = index; i + 1 < size_; ++i)
      data_[i] = std::move(data_[i + 1]);
    data_[size_ - 1].~T();
    --size_;
  }

  void pop_back() {
    if (size_ > 0) {
      --size_;
      data_[size_].~T();
    }
  }

  void clear() {
    destroy_all();
    size_ = 0;
  }

  void swap(Vector& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  // Non-member functions

  friend bool operator==(const Vector& lhs, const Vector& rhs) {
    if (lhs.size_ != rhs.size_) return false;
    for (std::size_t i = 0; i < lhs.size_; ++i) {
      if (!(lhs.data_[i] == rhs.data_[i])) return false;
    }
    return true;
  }

  friend bool operator!=(const Vector& lhs, const Vector& rhs) {
    return !(lhs == rhs);
  }

 private:
  // Internal helpers

  void destroy_all() noexcept {
    for (std::size_t i = 0; i < size_; ++i) data_[i].~T();
  }

  void reallocate(std::size_t new_capacity) {
    T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
    std::size_t constructed = 0;
    try {
      for (; constructed < size_; ++constructed) {
        ::new (static_cast<void*>(new_data + constructed))
            T(std::move(data_[constructed]));
      }
    } catch (...) {
      for (std::size_t i = 0; i < constructed; ++i) new_data[i].~T();
      ::operator delete(new_data);
      throw;
    }
    destroy_all();
    ::operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
  }

  T* data_ = nullptr;
  std::size_t size_ = 0;
  std::size_t capacity_ = 0;
};

}  // namespace cpplib
