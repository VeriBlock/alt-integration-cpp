// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/pop/mock_miner.hpp>
#include <veriblock/pop/serde.hpp>

#include "vector.hpp"

using namespace altintegration;
using namespace boost::python;

// for some reason const ref can't be returned to python.
// this helper gets const ref to hash, then returns a copy.
template <typename Block>
typename Block::hash_t ReturnHashCopy(Block& s) {
  return s.getHash();
}

boost::shared_ptr<Address> makeAddress(std::string s) {
  Address addr;
  ValidationState state;
  if (!addr.fromString(s, state)) {
    throw std::invalid_argument(state.toString());
  }
  return boost::shared_ptr<Address>(new Address(std::move(addr)));
}

list SerializePopData(const PopData& s) {
  list atvs;
  for (const auto& atv : s.atvs) {
    atvs.append(SerializeToHex(atv));
  }
  list vtbs;
  for (const auto& vtb : s.vtbs) {
    vtbs.append(SerializeToHex(vtb));
  }
  list context;
  for (const auto& block : s.context) {
    context.append(SerializeToHex(block));
  }
  list data;
  data.append(atvs);
  data.append(vtbs);
  data.append(context);
  return data;
}

void init_entities() {
  class_<PublicationData, boost::shared_ptr<PublicationData>>("PublicationData")
      .def("__str__", &PublicationData::toPrettyString)
      .def_readwrite("identifier", &PublicationData::identifier)
      .def_readwrite("header", &PublicationData::header)
      .def_readwrite("payoutInfo", &PublicationData::payoutInfo)
      .def_readwrite("contextInfo", &PublicationData::contextInfo);

  enum_<AddressType>("AddressType")
      .value("STANDARD", AddressType::STANDARD)
      .value("MULTISIG", AddressType::MULTISIG);

  class_<Address, boost::shared_ptr<Address>>("Address")
      .def("__init__", make_constructor(makeAddress))
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
      .def("__str__", &VbkPopTx::toPrettyString)
      .def("toVbkEncodingHex", &SerializeToHex<VbkPopTx>)
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
      .def("__str__", &Coin::toPrettyString)
      .def_readwrite("units", &Coin::units);

  class_<Output, boost::shared_ptr<Output>>("Output")
      .def("__str__", &Output::toPrettyString)
      .def_readwrite("address", &Output::address)
      .def_readwrite("coin", &Output::coin);

  class_<VbkTx, boost::shared_ptr<VbkTx>>("VbkTx")
      .def("getHash", &VbkTx::getHash)
      .def("toVbkEncodingHex", &SerializeToHex<VbkTx>)
      .def_readwrite("networkOrType", &VbkTx::networkOrType)
      .def_readwrite("sourceAddress", &VbkTx::sourceAddress)
      .def_readwrite("sourceAmount", &VbkTx::sourceAmount)
      .def_readwrite("outputs", &VbkTx::outputs)
      .def_readwrite("signatureIndex", &VbkTx::signatureIndex)
      .def_readwrite("publicationData", &VbkTx::publicationData)
      .def_readwrite("signature", &VbkTx::signature)
      .def_readwrite("publicKey", &VbkTx::publicKey);

  class_<VTB, boost::shared_ptr<VTB>>("VTB")
      .def("__str__", &VTB::toPrettyString)
      .def("toHex", &SerializeToHex<VTB>)
      .def("toVbkEncodingHex", &SerializeToHex<VTB>)
      .def("getId", &VTB::getId)
      .def_readwrite("transaction", &VTB::transaction)
      .def_readwrite("merklePath", &VTB::merklePath)
      .def_readwrite("containingBlock", &VTB::containingBlock);

  class_<ATV, boost::shared_ptr<ATV>>("ATV")
      .def("__str__", &ATV::toPrettyString)
      .def("toHex", &SerializeToHex<ATV>)
      .def("toVbkEncodingHex", &SerializeToHex<ATV>)
      .def("getId", &ATV::getId)
      .def_readwrite("transaction", &ATV::transaction)
      .def_readwrite("merklePath", &ATV::merklePath)
      .def_readwrite("blockOfProof", &ATV::blockOfProof);

  class_<BtcTx, boost::shared_ptr<BtcTx>>("BtcTx")
      .def("__str__", &SerializeToRawHex<BtcTx>)
      .def("getHash", &BtcTx::getHash)
      .def_readwrite("tx", &BtcTx::tx);

  class_<BtcBlock, boost::shared_ptr<BtcBlock>>("BtcBlock")
      .def("__str__", &BtcBlock::toPrettyString)
      .def("toHex", &SerializeToRawHex<BtcBlock>)
      .def("toVbkEncodingHex", &SerializeToHex<BtcBlock>)
      .def("getHash", &ReturnHashCopy<BtcBlock>)
      .add_property("version", &BtcBlock::getVersion, &BtcBlock::setVersion)
      .add_property("previousBlock",
                    &BtcBlock::getPreviousBlock,
                    &BtcBlock::setPreviousBlock)
      .add_property(
          "merkleRoot", &BtcBlock::getMerkleRoot, &BtcBlock::setMerkleRoot)
      .add_property(
          "timestamp", &BtcBlock::getTimestamp, &BtcBlock::setTimestamp)
      .add_property("bits", &BtcBlock::getDifficulty, &BtcBlock::setDifficulty)
      .add_property("nonce", &BtcBlock::getNonce, &BtcBlock::setNonce);

  class_<VbkBlock, boost::shared_ptr<VbkBlock>>("VbkBlock")
      .def("__str__", &VbkBlock::toPrettyString)
      .def("toHex", &SerializeToRawHex<VbkBlock>)
      .def("toVbkEncodingHex", &SerializeToHex<VbkBlock>)
      .def("getHash", &ReturnHashCopy<VbkBlock>)
      .add_property("height", &VbkBlock::getHeight, &VbkBlock::setHeight)
      .add_property("version", &VbkBlock::getVersion, &VbkBlock::setVersion)
      .add_property("previousBlock",
                    &VbkBlock::getPreviousBlock,
                    &VbkBlock::setPreviousBlock)
      .add_property("previousKeystone",
                    &VbkBlock::getPreviousKeystone,
                    &VbkBlock::setPreviousKeystone)
      .add_property("secondPreviousKeystone",
                    &VbkBlock::getSecondPreviousKeystone,
                    &VbkBlock::setSecondPreviousKeystone)
      .add_property(
          "merkleRoot", &VbkBlock::getMerkleRoot, &VbkBlock::setMerkleRoot)
      .add_property(
          "timestamp", &VbkBlock::getTimestamp, &VbkBlock::setTimestamp)
      .add_property(
          "difficulty", &VbkBlock::getDifficulty, &VbkBlock::setDifficulty)
      .add_property("nonce", &VbkBlock::getNonce, &VbkBlock::setNonce);

  class_<PopData, boost::shared_ptr<PopData>>("PopData")
      .def("__str__", &PopData::toPrettyString)
      .def("serialize", &SerializePopData)
      .def_readonly("atvs", &PopData::atvs)
      .def_readonly("vtbs", &PopData::vtbs)
      .def_readonly("context", &PopData::context);
}
