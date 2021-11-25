#ifndef B42704D4_8D00_44C6_9D3D_7EC2F1A19C81
#define B42704D4_8D00_44C6_9D3D_7EC2F1A19C81

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v4_range.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <unordered_map>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/time.hpp>

namespace altintegration {
namespace abfi {

namespace ip = boost::asio::ip;
using tcp = ip::tcp;

struct BanManager {
  struct Entry {
    uint32_t createTime;
    uint32_t banUntil;
  };

  bool isBanned(const ip::address& addr) const {
    auto time = currentTimestamp4();
    auto it = banned_.find(addr);
    if (it == banned_.end()) {
      return false;
    }

    const Entry& e = it->second;
    return time < e.banUntil;
  }

  void ban(const ip::address& addr, uint32_t banTime) {
    auto time = currentTimestamp4();
    auto it = banned_.find(addr);
    if (it == banned_.end()) {
      Entry e{time, time + banTime};
      banned_.insert(std::make_pair(addr, e));
      return;
    }

    Entry& e = it->second;
    e.banUntil += banTime;
  }

  //! clears banned if ban time passed
  void sweepBanned() {
    auto time = currentTimestamp4();
    for (auto it = banned_.begin(), end = banned_.end(); it != end;) {
      const ip::address& addr = it->first;
      const Entry& e = it->second;
      if (time > e.banUntil) {
        it = banned_.erase(it);
        VBK_LOG_INFO("peer %s is removed from banlist", addr.to_string());
      } else {
        ++it;
      }
    }
  }

 private:
  std::unordered_map<ip::address, Entry> banned_;
};
}  // namespace abfi
}  // namespace altintegration

#endif /* B42704D4_8D00_44C6_9D3D_7EC2F1A19C81 */
