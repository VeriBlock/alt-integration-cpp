#ifndef DF0DBD92_6F51_441B_B579_177C0CCC2AB1
#define DF0DBD92_6F51_441B_B579_177C0CCC2AB1

#include <boost/asio.hpp>
#include <cstdint>

namespace altintegration {
namespace abfi {

namespace ip = boost::asio::ip;
using tcp = ip::tcp;

using NodeId = uint64_t;

//! generic (protocol-agnostic) node
template <typename SessionT>
struct Node {
  // were we able to successfully connect+acknowledge this node?
  bool successfullyConnected = false;
  // if true, this node is inbound, else outbound
  bool inbound = false;
  // if true, peer should be disconnected next time when ConnectionManager calls
  // DisconnectNodes()
  bool disconnect = false;
  // Node ID
  NodeId id;
  // node address+port
  tcp::endpoint endpoint;
  // if we are connected to this peer, session will be set
  std::shared_ptr<SessionT> session = nullptr;
};

}  // namespace abfi
}  // namespace altintegration

#endif /* DF0DBD92_6F51_441B_B579_177C0CCC2AB1 */
