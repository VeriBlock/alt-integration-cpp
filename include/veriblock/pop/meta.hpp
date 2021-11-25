#ifndef E3980298_8DBC_4CA6_8D86_C73A21402EF9
#define E3980298_8DBC_4CA6_8D86_C73A21402EF9

#include <type_traits>

namespace altintegration {

//! value is equal to non-0 if U is derived from T
//! https://coderedirect.com/questions/222038/checking-if-a-class-inherits-from-any-template-instantiation-of-a-template
template <template <class> class T, class U>
struct IsDerivedFrom {
 private:
  template <class V>
  static decltype(static_cast<const T<V>&>(std::declval<U>()), std::true_type{})
      test(const T<V>&);

  static std::false_type test(...);

 public:
  static constexpr bool value =
      decltype(IsDerivedFrom::test(std::declval<U>()))::value;
};

}  // namespace altintegration

#endif /* E3980298_8DBC_4CA6_8D86_C73A21402EF9 */
