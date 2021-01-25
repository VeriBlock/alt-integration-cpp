// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner_2.hpp>

using namespace boost::python;
using namespace altintegration;

struct Payloads {
  ATV atv;
  std::vector<VTB> vtbs;
  std::vector<VbkBlock> context;

  std::string toPrettyString() const {
    return fmt::sprintf("Payloads(atv, vtbs=%d)", vtbs.size());
  }

  list prepare() const {
    list args;

    list listctx;
    list listvtbs;
    for (const auto& vtb : vtbs) {
      listvtbs.append(HexStr(vtb.toVbkEncoding()));
    }

    for (const auto& b : context) {
      listctx.append(HexStr(b.toVbkEncoding()));
    }

    list listatvs;
    listatvs.append(HexStr(atv.toVbkEncoding()));

    args.append(listctx);
    args.append(listvtbs);
    args.append(listatvs);
    return args;
  }
};

struct MockMiner2Proxy : private MockMiner2 {
  using base = MockMiner2;

  std::string toPrettyString() {
    std::ostringstream ss;
    ss << "MockMiner2(btc=" << this->getBtcTip().toPrettyString();
    ss << ", vbk=" << this->getVbkTip().toPrettyString();
    ss << ")";
    return ss.str();
  }

  VbkBlock getVbkTip() { return vbk().getBestChain().tip()->getHeader(); }
  BtcBlock getBtcTip() { return btc().getBestChain().tip()->getHeader(); }

  VbkBlock getVbkBlock(const std::string& hash) {
    auto index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(hash));
    if (!index) {
      throw std::invalid_argument("Can not find VbkBlock " + hash);
    }

    return index->getHeader();
  }

  BtcBlock getBtcBlock(const std::string& hash) {
    auto index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(hash));
    if (!index) {
      throw std::invalid_argument("Can not find BtcBlock " + hash);
    }

    return index->getHeader();
  }

  VbkBlock mineVbkBlocks(const std::string& prevHash, size_t num) {
    auto* index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner2: can't find prev block for mining: " +
                             prevHash);
    }

    return base::mineVbkBlocks(num, *index)->getHeader();
  }
  BtcBlock mineBtcBlocks(const std::string& prevHash, size_t num) {
    auto* index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner2: can't find prev block for mining: " +
                             prevHash);
    }

    return base::mineBtcBlocks(num, *index)->getHeader();
  }
  VbkBlock mineVbkBlocks(size_t num) {
    auto* index = vbk().getBestChain().tip();
    return base::mineVbkBlocks(num, *index)->getHeader();
  }
  BtcBlock mineBtcBlocks(size_t num) {
    auto* index = btc().getBestChain().tip();
    return base::mineBtcBlocks(num, *index)->getHeader();
  }

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) {
    return base::createBtcTxEndorsingVbkBlock(publishedBlock);
  }

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& containingBlock,
      const BtcTx& containingTx,
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash) {
    return base::createVbkPopTxEndorsingVbkBlock(
      containingBlock, 
      containingTx, 
      publishedBlock, 
      lastKnownBtcBlockHash);
  }

  VbkTx createVbkTxEndorsingAltBlock(const PublicationData& publicationData) {
    return base::createVbkTxEndorsingAltBlock(publicationData);
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
          "MockMiner2: endorseVbkBlock - can not find prevVbkIndex: " + prevVbk);
    }

    auto* prevBtcIndex =
        btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevBtc));
    if (!prevBtcIndex) {
      throw std::logic_error(
          "MockMiner2: envdorseVbkBlock - can not find prevBtcIndex: " +
          prevBtc);
    }

    auto lastBtc = prevBtcIndex->getHash().toHex();
    for (size_t i = 0; i < vtbs; i++) {
      auto btctx = createBtcTxEndorsingVbkBlock(block);
      auto btccontaining = this->mineBtcBlocks(lastBtc, 1);
      auto vbkpoptx = createVbkPopTxEndorsingVbkBlock(
          btccontaining,
          btctx,
          block,
          BtcBlock::hash_t::fromHex(lastKnownBtcHash));
      lastBtc = btccontaining.getHash().toHex();
    }

    this->mineVbkBlocks(prevVbkIndex->getHash().toHex(), 1);
  }

  Payloads endorseAltBlock(const PublicationData& pub,
                           const std::string& lastVbkBlock) {
    Payloads payloads;
    auto vbkindex =
        vbk().getBlockIndex(VbkBlock::hash_t::fromHex(lastVbkBlock));
    if (!vbkindex) {
      throw std::logic_error(
          "MockMiner2: endorseAltBlock called with unknown lastVbkBlock: " +
          lastVbkBlock);
    }
    auto vbktx = base::createVbkTxEndorsingAltBlock(pub);
    auto* vbkblock = base::mineVbkBlocks(1, {vbktx});
    payloads.atv = base::getATVs(*vbkblock)[0];

    std::vector<VbkBlock> context;
    // in range of blocks [lastVbkBlock... vbk tip] look for VTBs and put them
    // into Payloads
    auto vbktip = vbk().getBestChain().tip();
    auto last =
        vbkindex->pprev ? vbkindex->pprev->getHash() : vbkindex->getHash();
    auto& vtbs = payloads.vtbs;
    while (vbktip->getHash() != last) {
      auto it = vbkPayloads.find(vbktip->getHash());
      if (it != vbkPayloads.end()) {
        // insert in reverse order
        std::for_each(it->second.rbegin(),
                      it->second.rend(),
                      [&](const VTB& vtb) { vtbs.push_back(vtb); });
      }
      context.push_back(vbktip->getHeader());
      vbktip = vbktip->pprev;
    }
    std::reverse(vtbs.begin(), vtbs.end());

    std::reverse(context.begin(), context.end());
    payloads.context = context;

    return payloads;
  }
};

boost::shared_ptr<MockMiner2Proxy> makeMiner2() {
  return boost::shared_ptr<MockMiner2Proxy>(new MockMiner2Proxy());
}

void init_primitives();
void init_entities();

BOOST_PYTHON_MODULE(pypopminer2) {
  init_primitives();
  init_entities();

  class_<Payloads>("Payloads")
      .def("__repr__", &Payloads::toPrettyString)
      .def("prepare", &Payloads::prepare)
      .def_readonly("atv", &Payloads::atv)
      .def_readonly("vtbs", &Payloads::vtbs);

  // required to deal with function overloading
  BtcBlock (MockMiner2Proxy::*fx1)(const std::string&, size_t) =
      &MockMiner2Proxy::mineBtcBlocks;
  BtcBlock (MockMiner2Proxy::*fx2)(size_t) = &MockMiner2Proxy::mineBtcBlocks;
  VbkBlock (MockMiner2Proxy::*fx3)(const std::string&, size_t) =
      &MockMiner2Proxy::mineVbkBlocks;
  VbkBlock (MockMiner2Proxy::*fx4)(size_t) = &MockMiner2Proxy::mineVbkBlocks;

  void (MockMiner2Proxy::*fx5)(
      const VbkBlock& block, const std::string& lastKnownBtcHash, size_t) =
      &MockMiner2Proxy::endorseVbkBlock;
  void (MockMiner2Proxy::*fx6)(const VbkBlock& block,
                              const std::string& prevVbk,
                              const std::string& prevBtc,
                              const std::string& lastKnownBtcHash,
                              size_t) = &MockMiner2Proxy::endorseVbkBlock;

  class_<MockMiner2Proxy, boost::noncopyable, boost::shared_ptr<MockMiner2Proxy>>(
      "MockMiner2", no_init)
      .def("__init__", make_constructor(makeMiner2))
      .def("__repr__", &MockMiner2Proxy::toPrettyString)
      .def_readonly(
          "vbkTip", &MockMiner2Proxy::getVbkTip, "Current BtcTip in MockMiner2")
      .def_readonly(
          "btcTip", &MockMiner2Proxy::getBtcTip, "Current VbkTip in MockMiner2")
      .def("getBtcBlock",
           &MockMiner2Proxy::getBtcBlock,
           "Given block hash, returns BtcBlock if it exists, throws otherwise")
      .def("getVbkBlock",
           &MockMiner2Proxy::getVbkBlock,
           "Given block hash, returns VbkBlock if it exists, throws otherwise")
      .def("mineBtcBlocks", fx1)
      .def("mineBtcBlocks", fx2)
      .def("mineVbkBlocks", fx3)
      .def("mineVbkBlocks", fx4)
      .def("createBtcTxEndorsingVbkBlock", &MockMiner2Proxy::createBtcTxEndorsingVbkBlock)
      .def("createVbkPopTxEndorsingVbkBlock", &MockMiner2Proxy::createVbkPopTxEndorsingVbkBlock)
      .def("createVbkTxEndorsingAltBlock", &MockMiner2Proxy::createVbkTxEndorsingAltBlock)
      .def("endorseVbkBlock", fx5)
      .def("endorseVbkBlock", fx6)
      .def("endorseAltBlock", &MockMiner2Proxy::endorseAltBlock);
}