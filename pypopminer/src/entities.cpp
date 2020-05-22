// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner.hpp>

using namespace altintegration;
using namespace boost::python;

void init_entities() {
  class_<PublicationData>("PublicationData")
      .def("__repr__", &PublicationData::toPrettyString)
      .def_readwrite("identifier", &PublicationData::identifier)
      .def_readwrite("header", &PublicationData::header)
      .def_readwrite("payoutInfo", &PublicationData::payoutInfo)
      .def_readwrite("contextInfo", &PublicationData::contextInfo);

  class_<ATV>("ATV", no_init)
      .def("__str__", &ATV::toHex)
      .def("__repr__", &ATV::toPrettyString)
      .def("toHex", &ATV::toHex);

  class_<VTB>("VTB", no_init)
      .def("__str__", &VTB::toHex)
      .def("__repr__", &VTB::toPrettyString)
      .def("toHex", &VTB::toHex);

  class_<BtcTx>("BtcTx")
      .def("__str__", &BtcTx::toHex)
      .def("__repr__", &BtcTx::toHex)
      .def("getHash", &BtcTx::getHash)
      .def_readwrite("tx", &BtcTx::tx);

  class_<BtcBlock>("BtcBlock")
      .def("__str__", &BtcBlock::toHex)
      .def("__repr__", &BtcBlock::toPrettyString)
      .def("toHex", &BtcBlock::toHex)
      .def("getHash", &BtcBlock::getHash)
      .def_readwrite("version", &BtcBlock::version)
      .def_readwrite("previousBlock", &BtcBlock::previousBlock)
      .def_readwrite("merkleRoot", &BtcBlock::merkleRoot)
      .def_readwrite("timestamp", &BtcBlock::timestamp)
      .def_readwrite("bits", &BtcBlock::bits)
      .def_readwrite("nonce", &BtcBlock::nonce);

  class_<VbkBlock>("VbkBlock")
      .def("__str__", &VbkBlock::toHex)
      .def("__repr__", &VbkBlock::toPrettyString)
      .def("toHex", &VbkBlock::toHex)
      .def("getHash", &VbkBlock::getHash)
      .def_readwrite("height", &VbkBlock::height)
      .def_readwrite("version", &VbkBlock::version)
      .def_readwrite("previousBlock", &VbkBlock::previousBlock)
      .def_readwrite("previousKeystone", &VbkBlock::previousKeystone)
      .def_readwrite("secondPreviousKeystone",
                     &VbkBlock::secondPreviousKeystone)
      .def_readwrite("merkleRoot", &VbkBlock::merkleRoot)
      .def_readwrite("timestamp", &VbkBlock::timestamp)
      .def_readwrite("difficulty", &VbkBlock::difficulty)
      .def_readwrite("nonce", &VbkBlock::nonce);
}