// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/python.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/btctx.hpp>
#include <veriblock/mock_miner.hpp>
#include <veriblock/uint.hpp>

BOOST_PYTHON_MODULE(_entities) {
  using namespace boost::python;
  using namespace altintegration;

  def("uint256_fromHex", &uint256::fromHex);
  class_<BtcTx>("BtcTx").def_readwrite("tx", &BtcTx::tx);

  class_<uint256>("uint256").def("toHex", &uint256::toHex);

  def("BtcBlock_fromHex", &BtcBlock::fromHex);
  class_<BtcBlock>("BtcBlock")
      .def_readwrite("version", &BtcBlock::version)
      .def_readwrite("previousBlock", &BtcBlock::previousBlock)
      .def_readwrite("merkleRoot", &BtcBlock::merkleRoot)
      .def_readwrite("timestamp", &BtcBlock::timestamp)
      .def_readwrite("bits", &BtcBlock::bits)
      .def_readwrite("nonce", &BtcBlock::nonce)
      .def("toHex", &BtcBlock::toHex);
}