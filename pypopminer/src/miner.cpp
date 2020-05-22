// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner.hpp>

using namespace boost::python;
using namespace altintegration;

struct Payloads {
  ATV atv;
  std::vector<VTB> vtbs;

  std::string getAtv() const { return atv.toHex(); }

  list getVtbs() const {
    list l;
    for (auto& v : vtbs) {
      l.append(v.toHex());
    }
    return l;
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

  VbkBlock mineVbkBlocks(const std::string& prevHash, size_t num) {
    auto* index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash);
    }

    return *base::mineVbkBlocks(*index, num)->header;
  }
  BtcBlock mineBtcBlocks(const std::string& prevHash, size_t num) {
    auto* index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash);
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
                       const std::string& lastKnownBtcHash,
                       size_t vtbs = 1) {
    return endorseVbkBlock(block,
                           getVbkTip().getHash().toHex(),
                           getBtcTip().getHash().toHex(),
                           lastKnownBtcHash,
                           vtbs);
  }

  void endorseVbkBlock(const VbkBlock& block,
                       const std::string& prevVbk,
                       const std::string& prevBtc,
                       const std::string& lastKnownBtcHash,
                       size_t vtbs = 1) {
    auto* prevVbkIndex =
        vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevVbk));
    if (!prevVbkIndex) {
      throw std::logic_error(
          "MockMiner: endorseVbkBlock - can not find prevVbkIndex: " + prevVbk);
    }

    auto* prevBtcIndex =
        btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevBtc));
    if (!prevBtcIndex) {
      throw std::logic_error(
          "MockMiner: envdorseVbkBlock - can not find prevBtcIndex: " +
          prevBtc);
    }

    for (size_t i = 0; i < vtbs; i++) {
      auto btctx = createBtcTxEndorsingVbkBlock(block);
      auto btccontaining =
          this->mineBtcBlocks(prevBtcIndex->getHash().toHex(), 1);
      auto vbkpoptx = createVbkPopTxEndorsingVbkBlock(
          btccontaining,
          btctx,
          block,
          BtcBlock::hash_t::fromHex(lastKnownBtcHash));
    }

    this->mineVbkBlocks(prevVbkIndex->getHash().toHex(), 1);
  }

  Payloads endorseAltBlock(const PublicationData& pub,
                           const std::string& lastVbkBlock) {
    Payloads payloads;
    ValidationState state;
    auto vbkindex =
        vbk().getBlockIndex(VbkBlock::hash_t::fromHex(lastVbkBlock));
    if (!vbkindex) {
      throw std::logic_error(
          "MockMiner: endorseAltBlock called with unknown lastVbkBlock: " +
          lastVbkBlock);
    }
    auto vbktx = base::createVbkTxEndorsingAltBlock(pub);
    payloads.atv = base::generateATV(vbktx, vbkindex->getHash(), state);
    if (!state.IsValid()) {
      throw std::logic_error("MockMiner: can't create ATV: " +
                             state.toString());
    }

    std::vector<VbkBlock> context;
    // in range of blocks [lastVbkBlock... vbk tip] look for VTBs and put them
    // into Payloads
    auto vbktip = vbk().getBestChain().tip();
    auto last =
        vbkindex->pprev ? vbkindex->pprev->getHash() : vbkindex->getHash();
    while (vbktip->getHash() != last) {
      auto it = vbkPayloads.find(vbktip->getHash());
      if (it != vbkPayloads.end()) {
        // insert in reverse order
        std::for_each(it->second.rbegin(),
                      it->second.rend(),
                      [&](const VTB& vtb) { payloads.vtbs.push_back(vtb); });
      }
      context.push_back(*vbktip->header);
      vbktip = vbktip->pprev;
    }
    std::reverse(payloads.vtbs.begin(), payloads.vtbs.end());

    std::reverse(context.begin(), context.end());
    if (payloads.vtbs.size() > 0) {
      // supply all vbk context into first VTB
      payloads.vtbs[0].context = context;
    }
    return payloads;
  }
};

boost::shared_ptr<MockMinerProxy> makeMiner() {
  return boost::shared_ptr<MockMinerProxy>(new MockMinerProxy());
}

void init_primitives();
void init_entities();

BOOST_PYTHON_MODULE(pypopminer) {
  init_primitives();
  init_entities();

  class_<Payloads>("Payloads")
      .add_property("atv", &Payloads::atv, &Payloads::getAtv)
      .add_property("vtbs", &Payloads::vtbs, &Payloads::getVtbs);

  // required to deal with function overloading
  BtcBlock (MockMinerProxy::*fx1)(const std::string&, size_t) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*fx2)(size_t) = &MockMinerProxy::mineBtcBlocks;
  VbkBlock (MockMinerProxy::*fx3)(const std::string&, size_t) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*fx4)(size_t) = &MockMinerProxy::mineVbkBlocks;

  void (MockMinerProxy::*fx5)(
      const VbkBlock& block, const std::string& lastKnownBtcHash, size_t) =
      &MockMinerProxy::endorseVbkBlock;
  void (MockMinerProxy::*fx6)(const VbkBlock& block,
                              const std::string& prevVbk,
                              const std::string& prevBtc,
                              const std::string& lastKnownBtcHash,
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