#ifndef ALT_INTEGRATION_VERIBLOCK_SLICE_HPP
#define ALT_INTEGRATION_VERIBLOCK_SLICE_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>

namespace VeriBlock {

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

  // NB: the SFINAE here uses .data() as a incomplete/imperfect proxy for the
  // requirement on Container to be a contiguous sequence container.
  template <
      class Container,
      typename = typename std::enable_if<
          sizeof(typename Container::value_type) == 1 &&
          std::is_convertible<typename Container::pointer, pointer>::value && 
          std::is_convertible<
              typename Container::pointer,
              decltype(std::declval<Container>().data())>::value>>
  constexpr Slice(Container &cont) noexcept : Slice(cont.data(), cont.size()) {}

  constexpr pointer data() const noexcept { return storage_; }

  constexpr size_t size() const noexcept { return size_; }

  constexpr reference operator[](index_type idx) const { return data()[idx]; }

  constexpr std::vector<value_type> asVector() const {
    return std::vector<value_type>{storage_, storage_ + size_};
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

  Slice<ElementType> reverse() const {
    Slice<ElementType> ret = *this;
    std::reverse(const_cast<value_type *>(ret.begin()),
                 const_cast<value_type *>(ret.end()));
    return ret;
  }

 private:
  pointer storage_;
  size_t size_;
};

}  // namespace VeriBlock

#endif  //__SLICE__HPP__
