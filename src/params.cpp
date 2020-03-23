#include <veriblock/alt-util.hpp>
#include <veriblock/params.hpp>

namespace altintegration {

std::shared_ptr<BtcChainParams> makeBtcNetwork(const std::string& net) {
  if (net == "main") {
    return std::make_shared<BtcChainParamsMain>();
  } else if (net == "test") {
    return std::make_shared<BtcChainParamsTest>();
  } else if (net == "regtest") {
    return std::make_shared<BtcChainParamsRegTest>();
  } else {
    throw std::invalid_argument(
        "BTC network can be main/test/regtest, provided: " + net);
  }
}

std::shared_ptr<VbkChainParams> makeVbkNetwork(const std::string& net) {
  if (net == "main") {
    return std::make_shared<VbkChainParamsMain>();
  } else if (net == "test") {
    return std::make_shared<VbkChainParamsTest>();
  } else if (net == "alpha") {
    return std::make_shared<VbkChainParamsAlpha>();
  } else if (net == "regtest") {
    return std::make_shared<VbkChainParamsRegTest>();
  } else {
    throw std::invalid_argument(
        "VBK network can be main/test/alpha/regtest, provided: " + net);
  }
}
}  // namespace altintegration