#ifndef ALTINTEGRATION_PARAMS_HPP
#define ALTINTEGRATION_PARAMS_HPP

#include <stdexcept>
#include "veriblock/blockchain/btc_chain_params.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"

namespace altintegration {

std::shared_ptr<BtcChainParams> makeBtcNetwork(const std::string& net);
std::shared_ptr<VbkChainParams> makeVbkNetwork(const std::string& net);

}

#endif  // ALTINTEGRATION_PARAMS_HPP
