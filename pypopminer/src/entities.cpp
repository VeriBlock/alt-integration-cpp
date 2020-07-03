// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner.hpp>

#include "vector.hpp"

using namespace altintegration;
using namespace boost::python;

template <typename E>
std::string toVbkEncodingHex(E& self) {
  WriteStream w;
  self.toVbkEncoding(w);
  return HexStr(w.data());
}

void init_entities() {
  class_<PublicationData, boost::shared_ptr<PublicationData>>("PublicationData")
      .def("__repr__", &PublicationData::toPrettyString)
      .def_readwrite("identifier", &PublicationData::identifier)
      .def_readwrite("header", &PublicationData::header)
      .def_readwrite("payoutInfo", &PublicationData::payoutInfo)
      .def_readwrite("contextInfo", &PublicationData::contextInfo);

  enum_<AddressType>("AddressType")
      .value("ZERO_UNUSED", AddressType::ZERO_UNUSED)
      .value("STANDARD", AddressType::STANDARD)
      .value("PROOF_OF_PROOF", AddressType::PROOF_OF_PROOF)
      .value("MULTISIG", AddressType::MULTISIG);

  class_<Address, boost::shared_ptr<Address>>("Address", init<std::string>())
      .def("__repr__", &Address::toString)
      .def("__str__", &Address::toString)
      .def_readonly("type", &Address::getType);

  class_<MerklePath, boost::shared_ptr<MerklePath>>("MerklePath")
      .def_readwrite("index", &MerklePath::index)
      .def_readwrite("subject", &MerklePath::subject)
      .def_readwrite("layers", &MerklePath::layers);

  class_<VbkMerklePath, boost::shared_ptr<VbkMerklePath>>("VbkMerklePath")
      .def_readwrite("treeIndex", &VbkMerklePath::treeIndex)
      .def_readwrite("index", &VbkMerklePath::index)
      .def_readwrite("subject", &VbkMerklePath::subject)
      .def_readwrite("layers", &VbkMerklePath::layers);

  class_<VbkPopTx, boost::shared_ptr<VbkPopTx>>("VbkPopTx")
      .def("__repr__", &VbkPopTx::toPrettyString)
      .def("toVbkEncodingHex", &toVbkEncodingHex<VbkPopTx>)
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

  class_<Coin, boost::shared_ptr<Coin>>("Coin")
      .def("__repr__", &Coin::toPrettyString)
      .def_readwrite("units", &Coin::units);

  class_<Output, boost::shared_ptr<Output>>("Output")
      .def("__repr__", &Output::toPrettyString)
      .def_readwrite("address", &Output::address)
      .def_readwrite("coin", &Output::coin);

  class_<VbkTx, boost::shared_ptr<VbkTx>>("VbkTx")
      .def("getHash", &VbkTx::getHash)
      .def("toVbkEncodingHex", &toVbkEncodingHex<VbkTx>)
      .def_readwrite("networkOrType", &VbkTx::networkOrType)
      .def_readwrite("sourceAddress", &VbkTx::sourceAddress)
      .def_readwrite("sourceAmount", &VbkTx::sourceAmount)
      .def_readwrite("outputs", &VbkTx::outputs)
      .def_readwrite("signatureIndex", &VbkTx::signatureIndex)
      .def_readwrite("publicationData", &VbkTx::publicationData)
      .def_readwrite("signature", &VbkTx::signature)
      .def_readwrite("publicKey", &VbkTx::publicKey);

  class_<VTB, boost::shared_ptr<VTB>>("VTB")
      .def("__str__", &VTB::toHex)
      .def("__repr__", &VTB::toPrettyString)
      .def("toHex", &VTB::toHex)
      .def("toVbkEncodingHex", &toVbkEncodingHex<VTB>)
      .def("getId", &VTB::getId)
      .def_readwrite("transaction", &VTB::transaction)
      .def_readwrite("merklePath", &VTB::merklePath)
      .def_readwrite("containingBlock", &VTB::containingBlock)
      .def_readwrite("context", &VTB::context);

  class_<ATV, boost::shared_ptr<ATV>>("ATV")
      .def("__str__", &ATV::toHex)
      .def("__repr__", &ATV::toPrettyString)
      .def("toHex", &ATV::toHex)
      .def("toVbkEncodingHex", &toVbkEncodingHex<ATV>)
      .def("getId", &ATV::getId)
      .def_readwrite("transaction", &ATV::transaction)
      .def_readwrite("merklePath", &ATV::merklePath)
      .def_readwrite("containingBlock", &ATV::containingBlock)
      .def_readwrite("context", &ATV::context);

  class_<BtcTx, boost::shared_ptr<BtcTx>>("BtcTx")
      .def("__str__", &BtcTx::toHex)
      .def("__repr__", &BtcTx::toHex)
      .def("getHash", &BtcTx::getHash)
      .def_readwrite("tx", &BtcTx::tx);

  class_<BtcBlock, boost::shared_ptr<BtcBlock>>("BtcBlock")
      .def("__str__", &BtcBlock::toHex)
      .def("__repr__", &BtcBlock::toPrettyString)
      .def("toHex", &BtcBlock::toHex)
      .def("toVbkEncodingHex", &toVbkEncodingHex<BtcBlock>)
      .def("getHash", &BtcBlock::getHash)
      .def_readwrite("version", &BtcBlock::version)
      .def_readwrite("previousBlock", &BtcBlock::previousBlock)
      .def_readwrite("merkleRoot", &BtcBlock::merkleRoot)
      .def_readwrite("timestamp", &BtcBlock::timestamp)
      .def_readwrite("bits", &BtcBlock::bits)
      .def_readwrite("nonce", &BtcBlock::nonce);

  class_<VbkBlock, boost::shared_ptr<VbkBlock>>("VbkBlock")
      .def("__str__", &VbkBlock::toHex)
      .def("__repr__", &VbkBlock::toPrettyString)
      .def("toHex", &VbkBlock::toHex)
      .def("toVbkEncodingHex", &toVbkEncodingHex<VbkBlock>)
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