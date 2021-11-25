#ifndef B828B3B7_53F6_4842_A676_5A46D0C237D3
#define B828B3B7_53F6_4842_A676_5A46D0C237D3

#include <cstddef>
#include <cstdint>

namespace altintegration {
namespace abfi {
class CSipHasher {
 private:
  uint64_t v[4];
  uint64_t tmp;
  int count;

 public:
  /** Construct a SipHash calculator initialized with 128-bit key (k0, k1) */
  CSipHasher(uint64_t k0, uint64_t k1);
  /** Hash a 64-bit integer worth of data
   *  It is treated as if this was the little-endian interpretation of 8 bytes.
   *  This function can only be used when a multiple of 8 bytes have been
   * written so far.
   */
  CSipHasher& Write(uint64_t data);
  /** Hash arbitrary bytes. */
  CSipHasher& Write(const unsigned char* data, size_t size);
  /** Compute the 64-bit SipHash-2-4 of the data written so far. The object
   * remains untouched. */
  uint64_t Finalize() const;
};
}  // namespace abfi
}  // namespace altintegration

#endif /* B828B3B7_53F6_4842_A676_5A46D0C237D3 */
