#ifndef F3A5BDF2_5A17_446D_963A_F14C187D6633
#define F3A5BDF2_5A17_446D_963A_F14C187D6633

#include <veriblock/pop/uint.hpp>

#include "serialize.h"

namespace altintegration {
namespace abfi {
namespace btc {

/** getdata message type flags */
constexpr const uint32_t MSG_WITNESS_FLAG = 1 << 30;
constexpr const uint32_t MSG_TYPE_MASK = 0xffffffff >> 2;

/** getdata / inv message types.
 * These numbers are defined by the protocol. When adding a new value, be sure
 * to mention it in the respective BIP.
 */
enum GetDataMsg : uint32_t {
  UNDEFINED = 0,
  MSG_TX = 1,
  MSG_BLOCK = 2,
  MSG_WTX = 5,  //!< Defined in BIP 339
  // The following can only occur in getdata. Invs always use TX/WTX or BLOCK.
  MSG_FILTERED_BLOCK = 3,                            //!< Defined in BIP37
  MSG_CMPCT_BLOCK = 4,                               //!< Defined in BIP152
  MSG_WITNESS_BLOCK = MSG_BLOCK | MSG_WITNESS_FLAG,  //!< Defined in BIP144
  MSG_WITNESS_TX = MSG_TX | MSG_WITNESS_FLAG,        //!< Defined in BIP144
  // MSG_FILTERED_WITNESS_BLOCK is defined in BIP144 as reserved for future
  // use and remains unused.
  // MSG_FILTERED_WITNESS_BLOCK = MSG_FILTERED_BLOCK | MSG_WITNESS_FLAG,
};

/** inv message data */
struct Inv {
  uint32_t type = 0;
  uint256 hash{};

  Inv() = default;
  Inv(uint32_t t, const uint256& h) : type(t), hash(h) {}

  friend inline bool operator<(const Inv& a, const Inv& b) {
    return (a.type < b.type || (a.type == b.type && a.hash < b.hash));
  }

  SERIALIZE_METHODS(Inv, obj) { READWRITE(obj.type, obj.hash); }
};

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration

#endif /* F3A5BDF2_5A17_446D_963A_F14C187D6633 */
