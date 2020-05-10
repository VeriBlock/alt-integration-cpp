// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <veriblock/mock_miner.hpp>

using namespace boost::python;
using namespace altintegration;

struct Payloads {
  ATV atv;
  std::vector<VTB> vtbs;

  std::string toPrettyString() const {
    std::ostringstream os;
    os << "Payloads{";
    os << "ATV=" << atv.containingBlock.getHash().toHex();
    os << ", VTBs=" << vtbs.size() << "}";
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
  VbkBlock mineVbkBlocks(const std::string& prevHex, size_t num) {
    auto* index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevHex));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHex);
    }

    return *base::mineVbkBlocks(*index, num)->header;
  }
  BtcBlock mineBtcBlocks(const std::string& prevHex, size_t num) {
    auto* index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevHex));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHex);
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
  using base::createBtcTxEndorsingVbkBlock;
  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& containingBlock,
      const BtcTx& containingTx,
      const VbkBlock& publishedBlock,
      const std::string& lastKnownBtcBlockHashHex) {
    return base::createVbkPopTxEndorsingVbkBlock(
        containingBlock,
        containingTx,
        publishedBlock,
        BtcBlock::hash_t::fromHex(lastKnownBtcBlockHashHex));
  }

  Payloads endorseAltBlock(const PublicationData& pub,
                           const std::string& lastVbkBlockHex) {
    auto hash = VbkBlock::hash_t::fromHex(lastVbkBlockHex);
    return endorseAltBlock(pub, hash);
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
    if (!state.IsValid()) {
      throw std::logic_error("MockMiner: can't create ATV: " +
                             state.toString());
    }

    std::vector<VbkBlock> context;
    // in range of blocks [lastVbkBlock... vbk tip] look for VTBs and put them
    // into Payloads
    auto vbktip = vbk().getBestChain().tip();
    do {
      auto it = vbkPayloads.find(vbktip->getHash());
      if (it != vbkPayloads.end()) {
        // insert in reverse order
        payloads.vtbs.insert(
            payloads.vtbs.end(), it->second.rbegin(), it->second.rend());
      }
      context.push_back(*vbktip->header);
      vbktip = vbktip->pprev;
    } while (vbktip->getHash() != lastVbkBlock);

    std::reverse(payloads.vtbs.begin(), payloads.vtbs.end());
    std::reverse(context.begin(), context.end());
    if (!payloads.vtbs.empty()) {
      // supply all vbk context into first VTB
      payloads.vtbs[0].context = context;
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
  class_<blob_t, boost::shared_ptr<blob_t>>(name.c_str(), no_init)
      .def("__init__", make_constructor(makeBlob<N>))
      .def("__repr__", &Blob<N>::toHex)
      .def("__str__", &Blob<N>::toHex)
      .def("toHex", &Blob<N>::toHex);
}

boost::shared_ptr<std::vector<uint8_t>> vecFromHex(const str& s) {
  std::string hex = extract<std::string>(s);
  auto data = ParseHex(hex);
  return boost::shared_ptr<std::vector<uint8_t>>(
      new std::vector<uint8_t>(std::move(data)));
}

std::string vecToHex(const std::vector<uint8_t>& v) { return HexStr(v); }

BOOST_PYTHON_MODULE(_pypopminer) {
  blob<256 / 8>("uint256");
  blob<192 / 8>("uint192");
  blob<128 / 8>("uint128");
  blob<96 / 8>("uint96");
  blob<72 / 8>("uint72");

  class_<std::vector<uint8_t>, boost::shared_ptr<std::vector<uint8_t>>>(
      "ByteVector", no_init)
      .def("__init__", make_constructor(vecFromHex))
      .def("__str__", make_function(vecFromHex))
      .def("__repr__", make_function(vecToHex));

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
  BtcBlock (MockMinerProxy::*fx1)(const std::string&, size_t) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*fx2)(size_t) = &MockMinerProxy::mineBtcBlocks;
  VbkBlock (MockMinerProxy::*fx3)(const std::string&, size_t) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*fx4)(size_t) = &MockMinerProxy::mineVbkBlocks;

  Payloads (MockMinerProxy::*fx5)(const PublicationData&,
                                  const VbkBlock::hash_t&) =
      &MockMinerProxy::endorseAltBlock;
  Payloads (MockMinerProxy::*fx6)(const PublicationData&, const std::string&) =
      &MockMinerProxy::endorseAltBlock;

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
      .def("endorseAltBlock", fx5)
      .def("endorseAltBlock", fx6)
      .def("createVbkPopTxEndorsingVbkBlock",
           &MockMinerProxy::createVbkPopTxEndorsingVbkBlock)
      .def("createBtcTxEndorsingVbkBlock",
           &MockMinerProxy::createBtcTxEndorsingVbkBlock);
}