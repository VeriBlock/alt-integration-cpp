// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <list>

#include "../EntitiesProviders.hpp"
#include "../FuzzedDataProvider.hpp"
#include "rand.hpp"
#include "tree.hpp"

namespace ai = altintegration;

static const int hashsize = 8;

ai::MockMiner& APM() {
  static ai::MockMiner mm;
  return mm;
}

fuzz::Tree& TREE() {
  static fuzz::Tree tree;
  return tree;
}

struct AtvCandidate {
  ai::VbkTx tx;
  ai::VbkBlock blockOfProof;
};

static std::list<ai::VTB> vtbs;
static std::list<ai::ATV> atvs;
static std::list<AtvCandidate> atvcandidates;
static std::vector<ai::VbkTx> vbktxes;
static std::vector<ai::VbkPopTx> vbkpoptxes;
static std::vector<std::pair<ai::BtcTx, ai::VbkBlock>> btctxes;

enum class TxType {
  VBK_TX,
  VBK_POP_TX,

  kMaxValue = VBK_POP_TX
};

enum class Action {
  MINE_ALT,
  MINE_VBK,
  MINE_BTC,
  CREATE_BTC_TX,
  CREATE_VBK_TX,
  CREATE_VBK_POP_TX,
  SUBMIT_ATV,
  SUBMIT_VTB,
  SUBMIT_VBK,
  SUBMIT_ALL,

  kMaxValue = SUBMIT_ALL
};

enum class VbkSubmitType { ACTIVE_CHAIN, RANDOM, kMaxValue = RANDOM };

enum class ForkOption {
  NEXT_AFTER_TIP,
  RANDOM_BLOCK,
  ONE_OF_TIPS,
  kMaxValue = ONE_OF_TIPS
};

template <typename Tree>
const typename Tree::index_t* selectBlock(FuzzedDataProvider& p, Tree& tree) {
  auto option = p.ConsumeEnum<ForkOption>();
  switch (option) {
    case ForkOption::NEXT_AFTER_TIP:
      return tree.getBestChain().tip();
    case ForkOption::RANDOM_BLOCK: {
      const auto& blocks = tree.getBlocks();
      auto it = select_randomly(blocks.begin(), blocks.end());
      return it->second.get();
    }
    case ForkOption::ONE_OF_TIPS:
      const auto& tips = tree.getTips();
      auto it = select_randomly(tips.begin(), tips.end());
      return *it;
  }

  VBK_ASSERT(false);
}

fuzz::Block mineNextBlock(FuzzedDataProvider& p,
                          const ai::BlockIndex<ai::AltBlock>* index) {
  fuzz::Block block;
  block.popdata = TREE().popcontext->getMemPool().generatePopData();
  block.hash = p.ConsumeBytesOrFail<uint8_t>(hashsize);
  block.timestamp = index->getTimestamp() + 1;
  block.height = index->getHeight() + 1;
  block.prevhash = index->getHash();
  return block;
}

void handle(FuzzedDataProvider& p) {
  switch (p.ConsumeEnum<Action>()) {
    case Action::MINE_ALT: {
      // in ALT mine only chain
      auto* prev = TREE().popcontext->getAltBlockTree().getBlockIndex(TREE().bestBlock);
      VBK_ASSERT(prev != nullptr);
      auto block = mineNextBlock(p, prev);
      /* ignore=*/TREE().acceptBlock(block);
      break;
    }
    case Action::MINE_VBK: {
      auto* prev = selectBlock(p, APM().vbk());
      auto option = p.ConsumeEnum<TxType>();
      switch (option) {
        case TxType::VBK_TX: {
          auto* blockOfProof = APM().mineVbkBlocks(1, *prev, vbktxes);
          if (blockOfProof == nullptr) {
            vbktxes.pop_back();
            break;
          }
          for (auto& z : vbktxes) {
            AtvCandidate c;
            c.blockOfProof = blockOfProof->getHeader();
            c.tx = z;
            atvcandidates.push_back(std::move(c));
          }
          vbktxes.clear();
          break;
        }
        case TxType::VBK_POP_TX:
          auto* containing = APM().mineVbkBlocks(1, *prev, vbkpoptxes);
          if (containing == nullptr) {
            vbkpoptxes.pop_back();
            break;
          }
          for (auto& z : vbkpoptxes) {
            auto vtb = APM().createVTB(containing->getHeader(), z);
            vtbs.push_back(vtb);
          }
          vbkpoptxes.clear();
          break;
      }

      break;
    }
    case Action::MINE_BTC: {
      auto* prev = selectBlock(p, APM().btc());
      std::vector<ai::BtcTx> v;
      v.reserve(btctxes.size());
      for (const auto& z : btctxes) {
        v.push_back(z.first);
      }
      /* ignore=*/APM().mineBtcBlocks(1, *prev, v);
      btctxes.clear();
      break;
    }
    case Action::CREATE_BTC_TX: {
      auto* block = selectBlock(p, APM().vbk());
      auto tx = APM().createBtcTxEndorsingVbkBlock(block->getHeader());
      btctxes.emplace_back(tx, block->getHeader());
      break;
    }
    case Action::CREATE_VBK_TX: {
      auto* endorsedIndex =
          selectBlock(p, TREE().popcontext->getAltBlockTree());
      VBK_ASSERT(endorsedIndex);
      std::vector<uint8_t> mroot{1, 2, 3, 4, 5};
      auto* endorsedBlock = TREE().getBlock(endorsedIndex->getHash());
      VBK_ASSERT(endorsedBlock);
      ai::PublicationData pd;
      bool result =
          ai::GeneratePublicationData(endorsedIndex->getHash(),
                                      mroot,
                                      endorsedBlock->popdata,
                                      mroot,
                                      TREE().popcontext->getAltBlockTree(),
                                      pd);
      VBK_ASSERT(result);
      auto tx = APM().createVbkTxEndorsingAltBlock(pd);
      vbktxes.push_back(std::move(tx));
      break;
    }
    case Action::CREATE_VBK_POP_TX: {
      auto* block = selectBlock(p, APM().vbk());
      auto tx = APM().createVbkPopTxEndorsingVbkBlock(
          block->getHeader(), TREE().lastBtc().getHash());
      vbkpoptxes.push_back(tx);
      break;
    }
    case Action::SUBMIT_ATV: {
      if (atvcandidates.empty()) {
        break;
      }

      auto candidate = atvcandidates.front();
      atvcandidates.pop_front();

      auto atv = APM().createATV(candidate.blockOfProof, candidate.tx);
      ai::ValidationState state;
      /* ignore= */ TREE().popcontext->getMemPool().submit<ai::ATV>(atv, state);
      break;
    }
    case Action::SUBMIT_VTB: {
      if (vtbs.empty()) {
        break;
      }

      auto vtb = vtbs.front();
      vtbs.pop_front();

      ai::ValidationState state;
      /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VTB>(vtb, state);
      break;
    }
    case Action::SUBMIT_VBK: {
      auto option = p.ConsumeEnum<VbkSubmitType>();
      switch (option) {
        case VbkSubmitType::ACTIVE_CHAIN: {
          auto lastKnownVbk = TREE().lastVbk();
          auto* index = APM().vbk().getBlockIndex(lastKnownVbk.getHash());
          VBK_ASSERT(index);

          auto* fork = ai::findFork(APM().vbk().getBestChain(), index);
          VBK_ASSERT(fork);

          auto* tip = APM().vbk().getBestChain().tip();
          VBK_ASSERT(tip->getHeight() >= fork->getHeight());
          auto context =
              ai::getContext(APM().vbk(),
                             tip->getHash(),
                             tip->getHeight() - fork->getHeight() + 1);

          for (auto& c : context) {
            ai::ValidationState state;
            /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VbkBlock>(
                c, state);
          }

          break;
        }
        case VbkSubmitType::RANDOM: {
          const auto& blocks = APM().vbk().getBlocks();
          auto it = select_randomly(blocks.begin(), blocks.end());
          VBK_ASSERT(it != blocks.end());

          ai::ValidationState state;
          /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VbkBlock>(
              it->second->getHeader(), state);
          break;
        }
      }
      break;
    }
    case Action::SUBMIT_ALL: {
      // submit everything we have

      // all vbk blocks
      const auto& blocks = APM().vbk().getBlocks();
      for (auto& block : blocks) {
        ai::ValidationState state;
        /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VbkBlock>(
            block.second->getHeader(), state);
      }

      // submit all ATVs
      for (auto& a : atvs) {
        ai::ValidationState state;
        /* ignore= */ TREE().popcontext->getMemPool().submit<ai::ATV>(a, state);
      }
      atvs.clear();

      // submit all VTBs
      for (auto& a : vtbs) {
        ai::ValidationState state;
        /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VTB>(a, state);
      }
      vtbs.clear();

      for (auto& a : APM().getAllVTBs()) {
        for (auto& vtb : a.second) {
          ai::ValidationState state;
          /* ignore= */ TREE().popcontext->getMemPool().submit<ai::VTB>(vtb,
                                                                        state);
        }
      }
      break;
    }
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  FuzzedDataProvider p(Data, Size);
  srand(p.ConsumeIntegral<uint32_t>());
  bool hasEnoughData = true;

  do {
    try {
      handle(p);
    } catch (const NotEnoughDataException& e) {
      hasEnoughData = false;
    }
  } while (hasEnoughData);

  return 0;
}