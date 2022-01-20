#include <veriblock/pop/trace.hpp>

#if TRACY_ENABLE

#include <cstdlib>
// Overloading Global new operator
void* operator new(size_t sz)
{
  void* m = malloc(sz);
  TracySecureAlloc(m, sz);
  return m;
}
// Overloading Global delete operator
void operator delete(void* m) noexcept
{
  TracySecureFree(m);
  free(m);
}
// Overloading Global new[] operator
void* operator new[](size_t sz)
{
  void* m = malloc(sz);
  TracySecureAlloc(m, sz);
  return m;
}
// Overloading Global delete[] operator
void operator delete[](void* m) noexcept
{
  TracySecureFree(m);
  free(m);
}

#endif