#ifndef B4E43B7E_22EB_4696_9D57_51F165CE247F
#define B4E43B7E_22EB_4696_9D57_51F165CE247F

#include <boost/asio.hpp>
#include <veriblock/pop/fmt.hpp>

namespace altintegration {
namespace abfi {

namespace ip = boost::asio::ip;

inline std::string ToString(const ip::tcp::endpoint& endpoint) {
  return fmt::format("{}:{}", endpoint.address().to_string(), endpoint.port());
}

}  // namespace abfi
}  // namespace altintegration

#endif /* B4E43B7E_22EB_4696_9D57_51F165CE247F */
