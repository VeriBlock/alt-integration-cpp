// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/pop/mock_miner.hpp>

using namespace boost::python;
using namespace altintegration;

template <typename T>
inline std::vector<T> to_vector(const list& list) {
  return std::vector<T>(stl_input_iterator<T>(list), stl_input_iterator<T>());
}

struct MockMinerProxy : private MockMiner {
  using base = MockMiner;

  inline static AltChainParamsRegTest alt_param{};
  inline static VbkChainParamsRegTest vbk_param{};
  inline static BtcChainParamsRegTest btc_param{};

  MockMinerProxy() : MockMiner(alt_param, vbk_param, btc_param) {}

  std::string toPrettyString() {
    std::ostringstream ss;
    ss << "MockMiner(btc=" << this->btcTip().toPrettyString();
    ss << ", vbk=" << this->vbkTip().toPrettyString();
    ss << ")";
    return ss.str();
  }

  PopData endorseAltBlock(const PublicationData& publicationData,
                          const std::string& lastKnownVbkBlockHashHex) {
    const auto& lastKnownVbkBlockHash =
        VbkBlock::hash_t::fromHex(lastKnownVbkBlockHashHex);
    return base::endorseAltBlock(publicationData, lastKnownVbkBlockHash);
  }

  VTB endorseVbkBlock(const VbkBlock& publishedBlock,
                      const std::string& lastKnownBtcBlockHashHex) {
    const auto& lastKnownBtcBlockHash =
        BtcBlock::hash_t::fromHex(lastKnownBtcBlockHashHex);
    return base::endorseVbkBlock(publishedBlock, lastKnownBtcBlockHash);
  }

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const VbkBlock& publishedBlock,
      const std::string& lastKnownBtcBlockHashHex) {
    const auto& lastKnownBtcBlockHash =
        BtcBlock::hash_t::fromHex(lastKnownBtcBlockHashHex);
    return base::createVbkPopTxEndorsingVbkBlock(publishedBlock,
                                                 lastKnownBtcBlockHash);
  }

  PopData createPopDataEndorsingAltBlock(
      const VbkBlock& blockOfProof,
      const VbkTx& transaction,
      const std::string& lastKnownVbkBlockHashHex) {
    const auto& lastKnownVbkBlockHash =
        VbkBlock::hash_t::fromHex(lastKnownVbkBlockHashHex);
    return base::createPopDataEndorsingAltBlock(
        blockOfProof, transaction, lastKnownVbkBlockHash);
  }

  ATV createATV(const VbkBlock& blockOfProof, const VbkTx& transaction) const {
    return base::createATV(blockOfProof, transaction);
  }

  VbkTx createVbkTxEndorsingAltBlock(
      const PublicationData& publicationData) const {
    return base::createVbkTxEndorsingAltBlock(publicationData);
  }

  VTB createVTB(const VbkBlock& containingBlock,
                const VbkPopTx& transaction) const {
    return base::createVTB(containingBlock, transaction);
  }

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& blockOfProof,
      const BtcTx& transaction,
      const VbkBlock& publishedBlock,
      const std::string& lastKnownBtcBlockHashHex) const {
    const auto& lastKnownBtcBlockHash =
        BtcBlock::hash_t::fromHex(lastKnownBtcBlockHashHex);
    return base::createVbkPopTxEndorsingVbkBlock(
        blockOfProof, transaction, publishedBlock, lastKnownBtcBlockHash);
  }

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) const {
    return base::createBtcTxEndorsingVbkBlock(publishedBlock);
  }

  VbkBlock mineVbkBlocks(size_t amount) {
    return base::mineVbkBlocks(amount)->getHeader();
  }

  VbkBlock mineVbkBlocks(size_t amount,
                         const list& transactions,
                         bool pop = false) {
    if (pop) {
      return base::mineVbkBlocks(amount, to_vector<VbkPopTx>(transactions))
          ->getHeader();
    }
    return base::mineVbkBlocks(amount, to_vector<VbkTx>(transactions))
        ->getHeader();
  }

  VbkBlock mineVbkBlocks(size_t amount, const std::string& tipHashHex) {
    const auto& tipHash = VbkBlock::hash_t::fromHex(tipHashHex);
    const auto* tip = getVbkBlockIndex(tipHash);
    return base::mineVbkBlocks(amount, *tip)->getHeader();
  }

  VbkBlock mineVbkBlocks(size_t amount,
                         const std::string& tipHashHex,
                         const list& transactions,
                         bool pop = false) {
    const auto& tipHash = VbkBlock::hash_t::fromHex(tipHashHex);
    const auto* tip = getVbkBlockIndex(tipHash);
    if (pop) {
      return base::mineVbkBlocks(
                 amount, *tip, to_vector<VbkPopTx>(transactions))
          ->getHeader();
    }
    return base::mineVbkBlocks(amount, *tip, to_vector<VbkTx>(transactions))
        ->getHeader();
  }

  BtcBlock mineBtcBlocks(size_t amount) {
    return base::mineBtcBlocks(amount)->getHeader();
  }

  BtcBlock mineBtcBlocks(size_t amount, const list& transactions) {
    return base::mineBtcBlocks(amount, to_vector<BtcTx>(transactions))
        ->getHeader();
  }

  BtcBlock mineBtcBlocks(size_t amount, const std::string& tipHashHex) {
    const auto& tipHash = BtcBlock::hash_t::fromHex(tipHashHex);
    const auto* tip = getBtcBlockIndex(tipHash);
    return base::mineBtcBlocks(amount, *tip)->getHeader();
  }

  BtcBlock mineBtcBlocks(size_t amount,
                         const std::string& tipHashHex,
                         const list& transactions) {
    const auto& tipHash = BtcBlock::hash_t::fromHex(tipHashHex);
    const auto* tip = getBtcBlockIndex(tipHash);
    return base::mineBtcBlocks(amount, *tip, to_vector<BtcTx>(transactions))
        ->getHeader();
  }

  VbkBlock vbkTip() const { return base::vbkTip()->getHeader(); }

  BtcBlock btcTip() const { return base::btcTip()->getHeader(); }

  std::vector<VbkBlock> vbkTips() const {
    std::vector<VbkBlock> tips;
    for (auto it : vbk().getTips()) {
      tips.push_back(it->getHeader());
    }
    return tips;
  }

  std::vector<BtcBlock> btcTips() const {
    std::vector<BtcBlock> tips;
    for (auto it : btc().getTips()) {
      tips.push_back(it->getHeader());
    }
    return tips;
  }

  VbkBlock getAncestor(const VbkBlock& block, size_t height) {
    const auto* index = base::getVbkBlockIndex(block.getHash());
    const auto* ancestorIndex = index->getAncestor(height);
    VBK_ASSERT(ancestorIndex != nullptr);
    return ancestorIndex->getHeader();
  }

  BtcBlock getAncestor(const BtcBlock& block, size_t height) {
    const auto* index = base::getBtcBlockIndex(block.getHash());
    const auto* ancestorIndex = index->getAncestor(height);
    VBK_ASSERT(ancestorIndex != nullptr);
    return ancestorIndex->getHeader();
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

  VbkPopTx (MockMinerProxy::*createVbkPopTxEndorsingVbkBlock1)(
      const VbkBlock& publishedBlock,
      const std::string& lastKnownBtcBlockHashHex) =
      &MockMinerProxy::createVbkPopTxEndorsingVbkBlock;
  VbkPopTx (MockMinerProxy::*createVbkPopTxEndorsingVbkBlock2)(
      const BtcBlock& blockOfProof,
      const BtcTx& transaction,
      const VbkBlock& publishedBlock,
      const std::string& lastKnownBtcBlockHashHex) const =
      &MockMinerProxy::createVbkPopTxEndorsingVbkBlock;

  VbkBlock (MockMinerProxy::*mineVbkBlocks1)(size_t amount) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*mineVbkBlocks2)(
      size_t amount, const list& transactions, bool pop) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*mineVbkBlocks3)(size_t amount,
                                             const std::string& tipHash) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*mineVbkBlocks4)(size_t amount,
                                             const std::string& tipHash,
                                             const list& transactions,
                                             bool pop) =
      &MockMinerProxy::mineVbkBlocks;

  BtcBlock (MockMinerProxy::*mineBtcBlocks1)(size_t amount) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*mineBtcBlocks2)(
      size_t amount, const list& transactions) = &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*mineBtcBlocks3)(size_t amount,
                                             const std::string& tipHash) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*mineBtcBlocks4)(
      size_t amount, const std::string& tipHash, const list& transactions) =
      &MockMinerProxy::mineBtcBlocks;

  VbkBlock (MockMinerProxy::*getAncestor1)(
      const VbkBlock& block, size_t height) = &MockMinerProxy::getAncestor;
  BtcBlock (MockMinerProxy::*getAncestor2)(
      const BtcBlock& block, size_t height) = &MockMinerProxy::getAncestor;

  class_<MockMinerProxy, boost::noncopyable, boost::shared_ptr<MockMinerProxy>>(
      "MockMiner", no_init)
      .def("__init__", make_constructor(makeMiner))
      .def("__repr__", &MockMinerProxy::toPrettyString)
      .def("endorseAltBlock", &MockMinerProxy::endorseAltBlock)
      .def("endorseVbkBlock", &MockMinerProxy::endorseVbkBlock)
      .def("createVbkPopTxEndorsingVbkBlock", createVbkPopTxEndorsingVbkBlock1)
      .def("createPopDataEndorsingAltBlock",
           &MockMinerProxy::createPopDataEndorsingAltBlock)
      .def("createATV", &MockMinerProxy::createATV)
      .def("createVbkTxEndorsingAltBlock",
           &MockMinerProxy::createVbkTxEndorsingAltBlock)
      .def("createVTB", &MockMinerProxy::createVTB)
      .def("createVbkPopTxEndorsingVbkBlock", createVbkPopTxEndorsingVbkBlock2)
      .def("createBtcTxEndorsingVbkBlock",
           &MockMinerProxy::createBtcTxEndorsingVbkBlock)
      .def("mineVbkBlocks", mineVbkBlocks1)
      .def("mineVbkBlocks", mineVbkBlocks2)
      .def("mineVbkBlocks", mineVbkBlocks3)
      .def("mineVbkBlocks", mineVbkBlocks4)
      .def("mineBtcBlocks", mineBtcBlocks1)
      .def("mineBtcBlocks", mineBtcBlocks2)
      .def("mineBtcBlocks", mineBtcBlocks3)
      .def("mineBtcBlocks", mineBtcBlocks4)
      .def_readonly("vbkTip", &MockMinerProxy::vbkTip)
      .def_readonly("btcTip", &MockMinerProxy::btcTip)
      .def_readonly("vbkTips", &MockMinerProxy::vbkTips)
      .def_readonly("btcTips", &MockMinerProxy::btcTips)
      .def("getAncestor", getAncestor1)
      .def("getAncestor", getAncestor2);
}