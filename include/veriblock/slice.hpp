#ifndef __SLICE__HPP__
#define __SLICE__HPP__

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>

/// Non-owning contiguous array. Similar to gsl::span. Essentially, just a pair
/// T* + size_t
template <class ElementType>
struct Slice {
  using element_type = ElementType;
  using value_type = typename std::remove_cv<ElementType>::type;
  using index_type = std::ptrdiff_t;
  using pointer = element_type *;
  using reference = element_type &;

  using iterator = pointer;
  using const_iterator = const pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Slice() : storage_(nullptr), size_(0) {}

  explicit Slice(pointer ptr, size_t size) : storage_(ptr), size_(size) {}

  constexpr pointer data() const noexcept { return storage_; }

  constexpr size_t size() const noexcept { return size_; }

  constexpr reference operator[](index_type idx) const { return data()[idx]; }

  constexpr std::vector<value_type> asVector() const {
    return std::vector<ElementType>{storage_, storage_ + size_};
  }

  constexpr iterator begin() const noexcept { return storage_; }
  constexpr iterator end() const noexcept { return storage_ + size_; }
  constexpr iterator cbegin() const noexcept { return storage_; }
  constexpr iterator cend() const noexcept { return storage_ + size_; }
  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator{end()};
  }
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator{begin()};
  }
  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }
  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

 private:
  pointer storage_;
  size_t size_;
};

#endif  //__SLICE__HPP__
