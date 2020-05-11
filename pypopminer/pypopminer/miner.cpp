// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <iostream>
#include <veriblock/mock_miner.hpp>

#include "converters.hpp"

using namespace boost::python;
using namespace altintegration;

struct Payloads {
  bool hasAtv = false;
  ATV atv;
  list vtbs;

  std::string toPrettyString() const {
    std::ostringstream os;
    os << "Payloads{";
    os << "ATV="
       << (hasAtv ? atv.containingBlock.getHash().toHex() : "<empty>");
    os << ", VTBs=" << len(vtbs) << "}";
    return os.str();
  }
};

struct MockMinerProxy : private MockMiner {
  using base = MockMiner;

  std::string toPrettyString() {
    std::ostringstream ss;
    ss << "MockMiner{btc=" << this->getBtcTip().toPrettyString();
    ss << ", vbk=" << this->getVbkTip().toPrettyString();
    ss << "}";
    return ss.str();
  }

  VbkBlock getVbkTip() { return *vbk().getBestChain().tip()->header; }
  BtcBlock getBtcTip() { return *btc().getBestChain().tip()->header; }
  VbkBlock mineVbkBlocks(const VbkBlock::hash_t& prevHash, size_t num) {
    auto* index = vbk().getBlockIndex(prevHash);
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash.toHex());
    }

    return *base::mineVbkBlocks(*index, num)->header;
  }
  BtcBlock mineBtcBlocks(const BtcBlock::hash_t& prevHash, size_t num) {
    auto* index = btc().getBlockIndex(prevHash);
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash.toHex());
    }

    return *base::mineBtcBlocks(*index, num)->header;
  }
  VbkBlock mineVbkBlocks(size_t num) {
    auto* index = vbk().getBestChain().tip();
    return *base::mineVbkBlocks(*index, num)->header;
  }
  BtcBlock mineBtcBlocks(size_t num) {
    auto* index = btc().getBestChain().tip();
    return *base::mineBtcBlocks(*index, num)->header;
  }

  void endorseVbkBlock(const VbkBlock& block,
                       const BtcBlock::hash_t& lastKnownBtcHash,
                       size_t vtbs = 1) {
    return endorseVbkBlock(block,
                           getVbkTip().getHash(),
                           getBtcTip().getHash(),
                           lastKnownBtcHash,
                           vtbs);
  }

  void endorseVbkBlock(const VbkBlock& block,
                       const VbkBlock::hash_t& prevVbk,
                       const BtcBlock::hash_t& prevBtc,
                       const BtcBlock::hash_t& lastKnownBtcHash,
                       size_t vtbs = 1) {
    auto* prevVbkIndex = vbk().getBlockIndex(prevVbk);
    if (!prevVbkIndex) {
      throw std::logic_error(
          "MockMiner: endorseVbkBlock - can not find prevVbkIndex: " +
          prevVbk.toHex());
    }

    auto* prevBtcIndex = btc().getBlockIndex(prevBtc);
    if (!prevBtcIndex) {
      throw std::logic_error(
          "MockMiner: envdorseVbkBlock - can not find prevBtcIndex: " +
          prevBtc.toHex());
    }

    for (size_t i = 0; i < vtbs; i++) {
      auto btctx = createBtcTxEndorsingVbkBlock(block);
      auto btccontaining = this->mineBtcBlocks(prevBtc, 1);
      auto vbkpoptx = createVbkPopTxEndorsingVbkBlock(
          btccontaining, btctx, block, lastKnownBtcHash);
    }

    this->mineVbkBlocks(prevVbk, 1);
  }

  Payloads endorseAltBlock(const PublicationData& pub,
                           const VbkBlock::hash_t& lastVbkBlock) {
    Payloads payloads;
    ValidationState state;
    auto vbkindex = vbk().getBlockIndex(lastVbkBlock);
    if (!vbkindex) {
      throw std::logic_error(
          "MockMiner: endorseAltBlock called with unknown lastVbkBlock: " +
          lastVbkBlock.toHex());
    }
    auto vbktx = base::createVbkTxEndorsingAltBlock(pub);
    payloads.atv = base::generateATV(vbktx, lastVbkBlock, state);
    payloads.hasAtv = true;
    if (!state.IsValid()) {
      throw std::logic_error("MockMiner: can't create ATV: " +
                             state.toString());
    }

    std::vector<VbkBlock> context;
    // in range of blocks [lastVbkBlock... vbk tip] look for VTBs and put them
    // into Payloads
    auto vbktip = vbk().getBestChain().tip();
    auto last = vbkindex->pprev ? vbkindex->pprev->getHash() : lastVbkBlock;
    while (vbktip->getHash() != last) {
      auto it = vbkPayloads.find(vbktip->getHash());
      if (it != vbkPayloads.end()) {
        // insert in reverse order
        std::for_each(it->second.rbegin(),
                      it->second.rend(),
                      [&](const VTB& vtb) { payloads.vtbs.append(vtb); });
      }
      context.push_back(*vbktip->header);
      vbktip = vbktip->pprev;
    }

    payloads.vtbs.reverse();

    std::reverse(context.begin(), context.end());
    if (len(payloads.vtbs) > 0) {
      // supply all vbk context into first VTB
      VTB vtb = extract<VTB>(payloads.vtbs[0]);
      vtb.context = context;
      payloads.vtbs[0] = vtb;
    }
    return payloads;
  }
};

boost::shared_ptr<MockMinerProxy> makeMiner() {
  return boost::shared_ptr<MockMinerProxy>(new MockMinerProxy());
}

template <size_t N>
boost::shared_ptr<Blob<N>> makeBlob(const object& obj) {
  std::string hex = extract<std::string>(obj);
  auto b = Blob<N>::fromHex(hex);
  return boost::shared_ptr<Blob<N>>(new Blob<N>(std::move(b)));
}

template <size_t N>
void blob(std::string name) {
  using blob_t = Blob<N>;
  class_<blob_t, boost::noncopyable, boost::shared_ptr<blob_t>>(name.c_str(),
                                                                no_init)
      .def("__str__", &blob_t::toHex)
      .def("__len__", &blob_t::size)
      .def("__repr__", &blob_t::toHex)
      .def("toHex", &blob_t::toHex);

  pystring_converter().reg<blob_t>();
}

std::string vecToHex(std::vector<uint8_t>& v) { return HexStr(v); }

BOOST_PYTHON_MODULE(pypopminer) {
  class_<std::vector<uint8_t>,
         boost::noncopyable,
         boost::shared_ptr<std::vector<uint8_t>>>("ByteVector", no_init)
      .def("__str__", &vecToHex)
      .def("toHex", &vecToHex)
      .def("__len__", &std::vector<uint8_t>::size)
      .def("__repr__", &vecToHex);
  blob<256 / 8>("uint256");
  blob<192 / 8>("uint192");
  blob<128 / 8>("uint128");
  blob<96 / 8>("uint96");
  blob<72 / 8>("uint72");

  pystring_converter().reg<std::vector<uint8_t>>();

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

  class_<Payloads>("Payloads")
      .def("__repr__", &Payloads::toPrettyString)
      .def_readwrite("atv", &Payloads::atv)
      .def_readwrite("vtbs", &Payloads::vtbs);

  // required to deal with function overloading
  BtcBlock (MockMinerProxy::*fx1)(const BtcBlock::hash_t&, size_t) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*fx2)(size_t) = &MockMinerProxy::mineBtcBlocks;
  VbkBlock (MockMinerProxy::*fx3)(const VbkBlock::hash_t&, size_t) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*fx4)(size_t) = &MockMinerProxy::mineVbkBlocks;

  void (MockMinerProxy::*fx5)(
      const VbkBlock& block, const BtcBlock::hash_t& lastKnownBtcHash, size_t) =
      &MockMinerProxy::endorseVbkBlock;
  void (MockMinerProxy::*fx6)(const VbkBlock& block,
                              const VbkBlock::hash_t& prevVbk,
                              const BtcBlock::hash_t& prevBtc,
                              const BtcBlock::hash_t& lastKnownBtcHash,
                              size_t) = &MockMinerProxy::endorseVbkBlock;

  class_<MockMinerProxy, boost::noncopyable, boost::shared_ptr<MockMinerProxy>>(
      "MockMiner", no_init)
      .def("__init__", make_constructor(makeMiner))
      .def("__repr__", &MockMinerProxy::toPrettyString)
      .def_readonly("vbkTip", &MockMinerProxy::getVbkTip)
      .def_readonly("btcTip", &MockMinerProxy::getBtcTip)
      .def("mineBtcBlocks", fx1)
      .def("mineBtcBlocks", fx2)
      .def("mineVbkBlocks", fx3)
      .def("mineVbkBlocks", fx4)
      .def("endorseVbkBlock", fx5)
      .def("endorseVbkBlock", fx6)
      .def("endorseAltBlock", &MockMinerProxy::endorseAltBlock);
}