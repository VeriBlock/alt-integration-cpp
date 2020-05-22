// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/python.hpp>
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

  class_<Address>("Address")
      .def("__init__", make_constructor(&Address::fromString))
      .def("__repr__", &Address::toString)
      .def("__str__", &Address::toString);

  class_<MerklePath>("MerklePath")
      .def_readwrite("index", &MerklePath::index)
      .def_readwrite("subject", &MerklePath::subject)
      .def_readwrite("layers", &MerklePath::layers);

  class_<VbkMerklePath>("VbkMerklePath")
      .def_readwrite("treeIndex", &VbkMerklePath::treeIndex)
      .def_readwrite("index", &VbkMerklePath::index)
      .def_readwrite("subject", &VbkMerklePath::subject)
      .def_readwrite("layers", &VbkMerklePath::layers);

  class_<VbkPopTx>("VbkPopTx")
      .def("__repr__", &VbkPopTx::toPrettyString)
      .def("getHash", &VbkPopTx::getHash)
      .def_readwrite("networkOrType", &VbkPopTx::networkOrType)
      .def_readwrite("address", &VbkPopTx::address)
      .def_readwrite("publishedBlock", &VbkPopTx::publishedBlock)
      .def_readwrite("bitcoinTransaction", &VbkPopTx::bitcoinTransaction)
      .def_readwrite("merklePath", &VbkPopTx::merklePath)
      .def_readwrite("blockOfProof", &VbkPopTx::blockOfProof)
      .def_readwrite("blockOfProofContext", &VbkPopTx::blockOfProofContext)
      .def_readwrite("signature", &VbkPopTx::signature)
      .def_readwrite("publicKey", &VbkPopTx::publicKey);

  class_<Coin>("Coin")
      .def_readwrite("units", &Coin::units);

  class_<Output>("Output")
      .def_readwrite("address", &Output::address)
      .def_readwrite("coin", &Output::coin);

  class_<VbkTx>("VbkTx")
      .def("getHash", &VbkTx::getHash)
      .def_readwrite("networkOrType", &VbkTx::networkOrType)
      .def_readwrite("sourceAddress", &VbkTx::sourceAddress)
      .def_readwrite("sourceAmount", &VbkTx::sourceAmount)
      .def_readwrite("outputs", &VbkTx::outputs)
      .def_readwrite("signatureIndex", &VbkTx::signatureIndex)
      .def_readwrite("publicationData", &VbkTx::publicationData)
      .def_readwrite("signature", &VbkTx::signature)
      .def_readwrite("publicKey", &VbkTx::publicKey);

  class_<VTB>("VTB")
      .def("__str__", &VTB::toHex)
      .def("__repr__", &VTB::toPrettyString)
      .def("toHex", &VTB::toHex)
      .def("getId", &VTB::getId)
      .def_readwrite("transaction", &VTB::transaction)
      .def_readwrite("merklePath", &VTB::merklePath)
      .def_readwrite("containingBlock", &VTB::containingBlock)
      .def_readwrite("context", &VTB::context);

  class_<ATV>("ATV")
      .def("__str__", &ATV::toHex)
      .def("__repr__", &ATV::toPrettyString)
      .def("toHex", &ATV::toHex)
      .def("getId", &ATV::getId)
      .def_readwrite("transaction", &ATV::transaction)
      .def_readwrite("merklePath", &ATV::merklePath)
      .def_readwrite("containingBlock", &ATV::containingBlock)
      .def_readwrite("context", &ATV::context);
}