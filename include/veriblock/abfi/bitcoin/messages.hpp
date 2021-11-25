#ifndef D338DCF0_7B49_45BD_87BA_F304091D2D04
#define D338DCF0_7B49_45BD_87BA_F304091D2D04

#include <array>
#include <cstdint>
#include <veriblock/pop/write_stream.hpp>

namespace altintegration {
namespace abfi {
namespace btc {

extern const char *VERSION;
extern const char *VERACK;
extern const char *ADDR;
extern const char *INV;
extern const char *GETDATA;
extern const char *MERKLEBLOCK;
extern const char *GETBLOCKS;
extern const char *GETHEADERS;
extern const char *TX;
extern const char *HEADERS;
extern const char *BLOCK;
extern const char *GETADDR;
extern const char *MEMPOOL;
extern const char *PING;
extern const char *PONG;
extern const char *NOTFOUND;
extern const char *FILTERLOAD;
extern const char *FILTERADD;
extern const char *FILTERCLEAR;
extern const char *SENDHEADERS;
extern const char *FEEFILTER;
extern const char *SENDCMPCT;
extern const char *CMPCTBLOCK;
extern const char *GETBLOCKTXN;
extern const char *BLOCKTXN;

/** nServices flags */
enum ServiceFlags : uint64_t {
  // NOTE: When adding here, be sure to update serviceFlagToStr too
  // Nothing
  NODE_NONE = 0,
  // NODE_NETWORK means that the node is capable of serving the complete block
  // chain. It is currently
  // set by all Bitcoin Core non pruned nodes, and is unset by SPV clients or
  // other light clients.
  NODE_NETWORK = (1 << 0),
  // NODE_BLOOM means the node is capable and willing to handle bloom-filtered
  // connections.
  // Bitcoin Core nodes used to support this by default, without advertising
  // this bit,
  // but no longer do as of protocol version 70011 (= NO_BLOOM_VERSION)
  NODE_BLOOM = (1 << 2),
  // NODE_WITNESS indicates that a node can be asked for blocks and transactions
  // including
  // witness data.
  NODE_WITNESS = (1 << 3),
  // NODE_COMPACT_FILTERS means the node will service basic block filter
  // requests.
  // See BIP157 and BIP158 for details on how this is implemented.
  NODE_COMPACT_FILTERS = (1 << 6),
  // NODE_NETWORK_LIMITED means the same as NODE_NETWORK with the limitation of
  // only
  // serving the last 288 (2 day) blocks
  // See BIP159 for details on how this is implemented.
  NODE_NETWORK_LIMITED = (1 << 10),

  // Bits 24-31 are reserved for temporary experiments. Just pick a bit that
  // isn't getting used, or one not being used much, and notify the
  // bitcoin-development mailing list. Remember that service bits are just
  // unauthenticated advertisements, so your code must be robust against
  // collisions and other cases where nodes may be advertising a service they
  // do not actually support. Other service bits should be allocated via the
  // BIP process.
};

/**
 * Gets the set of service flags which are "desirable" for a given peer.
 *
 * These are the flags which are required for a peer to support for them
 * to be "interesting" to us, ie for us to wish to use one of our few
 * outbound connection slots for or for us to wish to prioritize keeping
 * their connection around.
 *
 * Relevant service flags may be peer- and state-specific in that the
 * version of the peer may determine which flags are required (eg in the
 * case of NODE_NETWORK_LIMITED where we seek out NODE_NETWORK peers
 * unless they set NODE_NETWORK_LIMITED and we are out of IBD, in which
 * case NODE_NETWORK_LIMITED suffices).
 *
 * Thus, generally, avoid calling with peerServices == NODE_NONE, unless
 * state-specific flags must absolutely be avoided. When called with
 * peerServices == NODE_NONE, the returned desirable service flags are
 * guaranteed to not change dependent on state - ie they are suitable for
 * use when describing peers which we know to be desirable, but for which
 * we do not have a confirmed set of service flags.
 *
 * If the NODE_NONE return value is changed, contrib/seeds/makeseeds.py
 * should be updated appropriately to filter for the same nodes.
 */
ServiceFlags GetDesirableServiceFlags(ServiceFlags services);

/**
 * A shortcut for (services & GetDesirableServiceFlags(services))
 * == GetDesirableServiceFlags(services), ie determines whether the given
 * set of service flags are sufficient for a peer to be "relevant".
 */
static inline bool HasAllDesirableServiceFlags(ServiceFlags services) {
  return !(GetDesirableServiceFlags(services) & (~services));
}

/**
 * Checks if a peer with the given service flags may be capable of having a
 * robust address-storage DB.
 */
static inline bool MayHaveUsefulAddressDB(ServiceFlags services) {
  return (services & NODE_NETWORK) || (services & NODE_NETWORK_LIMITED);
}

struct MessageHeader {
  static const uint32_t MAX_COMMAND_SIZE = 12;
  static const uint32_t HEADER_SIZE = 24;
  static const uint32_t MAX_SIZE = 0x02000000;
  static const constexpr std::array<uint8_t, 4> emptyChecksum{
      0x5d, 0xf6, 0xe0, 0xe2};

  std::array<uint8_t, 4> magic;
  std::array<uint8_t, MAX_COMMAND_SIZE> command;
  uint32_t payloadSize = 0;
  std::array<uint8_t, 4> checksum = emptyChecksum;
};

void Serialize(WriteStream &stream, const MessageHeader &header);

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration

#endif /* D338DCF0_7B49_45BD_87BA_F304091D2D04 */
