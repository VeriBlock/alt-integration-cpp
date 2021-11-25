#ifndef C983B3A8_20F1_4F8E_AF42_8D4EF78187CC
#define C983B3A8_20F1_4F8E_AF42_8D4EF78187CC

#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <memory>
#include <unordered_map>
#include <veriblock/pop/algorithm.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/meta.hpp>

#include "ban_manager.hpp"
#include "node.hpp"
#include "util.hpp"

namespace altintegration {
namespace abfi {

namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using errcode = boost::system::error_code;

//! @invariant server object lives longer than any of session objects
template <typename SharedStateT, typename SessionT>
struct ConnectionManager {
  static_assert(IsDerivedFrom<std::enable_shared_from_this, SessionT>::value,
                "SessionT must be derived from std::enable_shared_from_this");

  using NodeT = Node<SessionT>;

  ConnectionManager(boost::asio::io_context& context,
                    const tcp::endpoint& endpoint,
                    SharedStateT& state)
      : context_(context), acceptor_(context, endpoint), state_(state) {}

  void connect(const tcp::endpoint& endpoint) {
    if (isLocal(endpoint)) {
      // do not connect to local server
      VBK_LOG_DEBUG("skipping connection to a local server %s",
                    ToString(endpoint));
      return;
    }

    const NodeT* exists = findNode(endpoint);
    if (exists != nullptr) {
      VBK_LOG_DEBUG("failed to open new connection, already connected to %s",
                    ToString(endpoint));
      return;
    }

    if (banmgr_.isBanned(endpoint.address())) {
      VBK_LOG_DEBUG("failed to open new connection, peer banned %s",
                    ToString(endpoint));
      return;
    }

    // create outbound session
    auto session = std::make_shared<SessionT>(
        /*context=*/context_,
        /*state=*/state_);

    // attempt to open new connection
    auto& socket = session->getSocket();
    socket.async_connect(
        endpoint, [this, session](errcode ec, tcp::resolver::iterator& it) {
          if (ec) {
            VBK_LOG_INFO("can not connect to %s: %s",
                         ToString(it->endpoint()),
                         ec.message());
            return;
          }

          auto node = make_unique<NodeT>();
          node->inbound = false;
          node->id = getNextNodeId();
          node->endpoint = it->endpoint();
          node->session = session;

          VBK_LOG_INFO(
              "node %d connected (%s)", node->id, ToString(it->endpoint()));

          // add node to a node list
          nodes_[node->id] = std::move(node);
        });
  }

  NodeId getNextNodeId() noexcept { return lastNodeId++; }

  void start() {
    stopped = false;
    accept();
  }

  void stop() { stopped = true; }

 private:
  void accept() {
    if (stopped) {
      return;
    }

    acceptor_.async_accept([this](errcode ec, tcp::socket socket) {
      if (ec) {
        VBK_LOG_ERROR("accept error: %s", ec.message());
        return;
      }

      const auto endpoint = socket.remote_endpoint();
      const auto* exists = findNode(endpoint);
      if (exists != nullptr) {
        VBK_LOG_INFO(
            "node %s attempted to connect for the second time, dropping "
            "connection.",
            ToString(endpoint));
        return;
      }

      if (banmgr_.isBanned(endpoint.address())) {
        VBK_LOG_DEBUG("failed to accept new connection, peer banned %s",
                      ToString(endpoint));
        return;
      }

      // it's a new node
      auto node = make_unique<NodeT>();
      node->inbound = true;
      node->id = getNextNodeId();
      node->endpoint = endpoint;
      node->session = std::make_shared<SessionT>(
          /*socket=*/std::move(socket),
          /*state=*/state_);
      node->session->startInbound();

      // add node to a node list
      nodes_[node->id] = std::move(node);
    });
  }

  const NodeT* findNode(const tcp::endpoint& endpoint) const {
    for (const auto& it : nodes_) {
      const NodeT* node = it->second;
      if (node->endpoint == endpoint) {
        return node;
      }
    }

    return nullptr;
  }

  bool isLocal(const tcp::endpoint& endpoint) const {
    return endpoint == acceptor_.local_endpoint();
  }

 private:
  NodeId lastNodeId = 0;
  boost::asio::io_context& context_;
  tcp::acceptor acceptor_;
  bool stopped = false;
  SharedStateT& state_;
  BanManager banmgr_;

  // all (inbound/outbound) connected nodes
  std::unordered_map<NodeId, std::unique_ptr<NodeT>> nodes_;
};

}  // namespace abfi
}  // namespace altintegration

#endif /* C983B3A8_20F1_4F8E_AF42_8D4EF78187CC */
