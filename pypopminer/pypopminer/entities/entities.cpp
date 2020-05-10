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

using namespace boost::python;
using namespace altintegration;

BOOST_PYTHON_MODULE(_entities) {
  class_<BtcTx>("BtcTx")
      .def("__str__", &BtcTx::toHex)
      .def("__repr__", &BtcTx::toHex)
      .def("getHash", &BtcTx::getHash)
      .def_readwrite("tx", &BtcTx::tx);

  class_<uint256>("uint256", no_init)
      .def("__str__", &uint256::toHex)
      .def("__repr__", &uint256::toHex)
      .def("__len__", &uint256::size)
      .def("toHex", &uint256::toHex);

  class_<BtcBlock>("BtcBlock")
      .def("__str__", &BtcBlock::toHex)
      .def("__repr__", &BtcBlock::toHex)
      .def("toHex", &BtcBlock::toHex)
      .def_readwrite("version", &BtcBlock::version)
      .def_readwrite("previousBlock", &BtcBlock::previousBlock)
      .def_readwrite("merkleRoot", &BtcBlock::merkleRoot)
      .def_readwrite("timestamp", &BtcBlock::timestamp)
      .def_readwrite("bits", &BtcBlock::bits)
      .def_readwrite("nonce", &BtcBlock::nonce);
}