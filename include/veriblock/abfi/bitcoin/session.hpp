#ifndef A4044B89_36A4_439A_8E05_CB177DD081F3
#define A4044B89_36A4_439A_8E05_CB177DD081F3

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/write.hpp>
#include <list>
#include <memory>
#include <veriblock/pop/abfi/common/node.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/write_stream.hpp>

#include "messages.hpp"
#include "state.hpp"

namespace altintegration {
namespace abfi {
namespace btc {

namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using errcode = boost::system::error_code;

struct Session : public std::enable_shared_from_this<Session> {
  // constructor for inbound session
  Session(tcp::socket socket, State& state)
      : socket_(std::move(socket)), state_(state) {
    startInbound();
  }

  // constructor for outbound session
  Session(boost::asio::io_context& context, State& state)
      : socket_(context), state_(state) {}

  bool isOpen() const noexcept { return socket_.is_open(); }

  void startInbound() {}

  void write(std::vector<uint8_t> message) {
    VBK_ASSERT(isOpen());
    bool writeInProgress = !write_queue_.empty();
    write_queue_.push_back(std::move(message));
    if (!writeInProgress) {
      doWrite();
    }
  }

  tcp::socket& getSocket() { return socket_; }

 private:
  void doWrite() {
    auto& front = write_queue_.front();
    auto _this = shared_from_this();
    boost::asio::async_write(socket_,
                             boost::asio::buffer(front.data(), front.size()),
                             [_this](errcode ec, size_t /*size*/) {
                               if (ec) {
                                 VBK_LOG_DEBUG("can not write: %s",
                                               ec.message());
                                 _this->socket_.close();
                                 return;
                               }

                               _this->write_queue_.pop_front();
                               if (!_this->write_queue_.empty()) {
                                 // continue async write chain
                                 _this->doWrite();
                               }
                             });
  }

 private:
  tcp::socket socket_;
  State& state_;

  std::list<std::vector<uint8_t>> write_queue_;
};

}  // namespace btc
}  // namespace abfi
}  // namespace altintegration

#endif /* A4044B89_36A4_439A_8E05_CB177DD081F3 */
