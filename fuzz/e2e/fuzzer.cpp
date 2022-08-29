// Copyright (c) 2019-2022 Xenios SEZC
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

struct AtvCandidate {
  ai::VbkTx tx;
  ai::VbkBlock blockOfProof;
};

struct FuzzState {
  ai::AltChainParamsRegTest altparam;
  ai::VbkChainParamsRegTest vbkparam;
  ai::BtcChainParamsRegTest btcparam;
  ai::MockMiner mm{altparam, vbkparam, btcparam};
  fuzz::Tree tree;
  std::list<ai::VTB> vtbs;
  std::list<ai::ATV> atvs;
  std::list<AtvCandidate> atvcandidates;
  std::vector<ai::VbkTx> vbktxes;
  std::vector<ai::VbkPopTx> vbkpoptxes;
  std::vector<std::pair<ai::BtcTx, ai::VbkBlock>> btctxes;

  ai::MockMiner& APM() { return mm; }

  fuzz::Tree& TREE() { return tree; }
};

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
      auto blocks = tree.getBlocks();
      auto it = select_randomly(blocks.begin(), blocks.end());
      return *it;
    }
    case ForkOption::ONE_OF_TIPS:
      const auto& tips = tree.getTips();
      auto it = select_randomly(tips.begin(), tips.end());
      return *it;
  }

  VBK_ASSERT(false);
}

fuzz::Block mineNextBlock(const ai::BlockIndex<ai::AltBlock>* index,
                          FuzzState& state) {
  fuzz::Block block;
  block.popdata = state.TREE().popcontext->getMemPool().generatePopData();
  std::generate_n(
      std::back_inserter(block.hash), hashsize, []() { return rand(); });
  block.timestamp = index->getTimestamp() + 1;
  block.height = index->getHeight() + 1;
  block.prevhash = index->getHash();
  return block;
}

bool handle(FuzzedDataProvider& p, FuzzState& state) {
  if (p.remaining_bytes() == 0) {
    return false;
  }

  switch (p.ConsumeEnum<Action>()) {
    case Action::MINE_ALT: {
      // in ALT mine only chain
      auto* prev = state.TREE().popcontext->getAltBlockTree().getBlockIndex(
          state.TREE().bestBlock);
      VBK_ASSERT(prev != nullptr);
      auto block = mineNextBlock(prev, state);
      /* ignore=*/state.TREE().acceptBlock(block);
      break;
    }
    case Action::MINE_VBK: {
      auto* prev = selectBlock(p, state.APM().vbk());
      auto option = p.ConsumeEnum<TxType>();
      switch (option) {
        case TxType::VBK_TX: {
          auto* blockOfProof =
              state.APM().mineVbkBlocks(1, *prev, state.vbktxes);
          if (blockOfProof == nullptr) {
            state.vbktxes.pop_back();
            break;
          }
          for (auto& z : state.vbktxes) {
            AtvCandidate c;
            c.blockOfProof = blockOfProof->getHeader();
            c.tx = z;
            state.atvcandidates.push_back(std::move(c));
          }
          state.vbktxes.clear();
          break;
        }
        case TxType::VBK_POP_TX:
          auto* containing =
              state.APM().mineVbkBlocks(1, *prev, state.vbkpoptxes);
          if (containing == nullptr) {
            state.vbkpoptxes.pop_back();
            break;
          }
          for (auto& z : state.vbkpoptxes) {
            auto vtb = state.APM().createVTB(containing->getHeader(), z);
            state.vtbs.push_back(vtb);
          }
          state.vbkpoptxes.clear();
          break;
      }

      break;
    }
    case Action::MINE_BTC: {
      auto* prev = selectBlock(p, state.APM().btc());
      std::vector<ai::BtcTx> v;
      v.reserve(state.btctxes.size());
      for (const auto& z : state.btctxes) {
        v.push_back(z.first);
      }
      /* ignore=*/state.APM().mineBtcBlocks(1, *prev, v);
      state.btctxes.clear();
      break;
    }
    case Action::CREATE_BTC_TX: {
      auto* block = selectBlock(p, state.APM().vbk());
      auto tx = state.APM().createBtcTxEndorsingVbkBlock(block->getHeader());
      state.btctxes.emplace_back(tx, block->getHeader());
      break;
    }
    case Action::CREATE_VBK_TX: {
      auto* endorsedIndex =
          selectBlock(p, state.TREE().popcontext->getAltBlockTree());
      VBK_ASSERT(endorsedIndex);
      std::vector<uint8_t> mroot{1, 2, 3, 4, 5};
      auto* endorsedBlock = state.TREE().getBlock(endorsedIndex->getHash());
      // if endorsedBlock is not found in TREE() then skip
      VBK_ASSERT_MSG(
          endorsedBlock != nullptr,
          "TREE:\n%s\n\nAltBlockTree:\n%s\n\n",
          state.TREE().toPrettyString(),
          state.TREE().popcontext->getAltBlockTree().toPrettyString());
      ai::PublicationData pd;
      bool result = ai::GeneratePublicationData(
          endorsedIndex->getHash(),
          mroot,
          endorsedBlock->popdata,
          mroot,
          state.TREE().popcontext->getAltBlockTree(),
          pd);
      VBK_ASSERT(result);
      auto tx = state.APM().createVbkTxEndorsingAltBlock(pd);
      state.vbktxes.push_back(std::move(tx));
      break;
    }
    case Action::CREATE_VBK_POP_TX: {
      auto* block = selectBlock(p, state.APM().vbk());
      auto tx = state.APM().createVbkPopTxEndorsingVbkBlock(
          block->getHeader(), state.TREE().lastBtc().getHash());
      state.vbkpoptxes.push_back(tx);
      break;
    }
    case Action::SUBMIT_ATV: {
      if (state.atvcandidates.empty()) {
        break;
      }

      auto candidate = state.atvcandidates.front();
      state.atvcandidates.pop_front();

      auto atv = state.APM().createATV(candidate.blockOfProof, candidate.tx);
      ai::ValidationState dummy;
      /* ignore= */ state.TREE().popcontext->getMemPool().submit<ai::ATV>(
          atv, true, dummy);
      break;
    }
    case Action::SUBMIT_VTB: {
      if (state.vtbs.empty()) {
        break;
      }

      auto vtb = state.vtbs.front();
      state.vtbs.pop_front();

      ai::ValidationState dummy;
      /* ignore= */ state.TREE().popcontext->getMemPool().submit<ai::VTB>(
          vtb, true, dummy);
      break;
    }
    case Action::SUBMIT_VBK: {
      auto option = p.ConsumeEnum<VbkSubmitType>();
      switch (option) {
        case VbkSubmitType::ACTIVE_CHAIN: {
          auto lastKnownVbk = state.TREE().lastVbk();
          auto* index = state.APM().vbk().getBlockIndex(lastKnownVbk.getHash());
          VBK_ASSERT(index);

          auto* fork = ai::findFork(state.APM().vbk().getBestChain(), index);
          VBK_ASSERT(fork);

          auto* tip = state.APM().vbk().getBestChain().tip();
          VBK_ASSERT(tip->getHeight() >= fork->getHeight());
          auto context =
              ai::getContext(state.APM().vbk(),
                             tip->getHash(),
                             tip->getHeight() - fork->getHeight() + 1);

          for (auto& c : context) {
            ai::ValidationState dummy;
            /* ignore= */ state.TREE()
                .popcontext->getMemPool()
                .submit<ai::VbkBlock>(c, true, dummy);
          }

          break;
        }
        case VbkSubmitType::RANDOM: {
          auto blocks = state.APM().vbk().getBlocks();
          auto it = select_randomly(blocks.begin(), blocks.end());
          VBK_ASSERT(it != blocks.end());

          ai::ValidationState dummy;
          /* ignore= */ state.TREE()
              .popcontext->getMemPool()
              .submit<ai::VbkBlock>((*it)->getHeader(), true, dummy);
          break;
        }
      }
      break;
    }
    case Action::SUBMIT_ALL: {
      // submit everything we have

      // all vbk blocks
      const auto& blocks = state.APM().vbk().getBlocks();
      for (auto& block : blocks) {
        ai::ValidationState dummy;
        /* ignore= */ state.TREE()
            .popcontext->getMemPool()
            .submit<ai::VbkBlock>(block->getHeader(), true, dummy);
      }

      // submit all ATVs
      for (auto& a : state.atvs) {
        ai::ValidationState dummy;
        /* ignore= */ state.TREE().popcontext->getMemPool().submit<ai::ATV>(
            a, true, dummy);
      }
      state.atvs.clear();

      // submit all VTBs
      for (auto& a : state.vtbs) {
        ai::ValidationState dummy;
        /* ignore= */ state.TREE().popcontext->getMemPool().submit<ai::VTB>(
            a, true, dummy);
      }
      state.vtbs.clear();

      for (auto& a : state.APM().getAllVTBs()) {
        for (auto& vtb : a.second) {
          ai::ValidationState dummy;
          /* ignore= */ state.TREE().popcontext->getMemPool().submit<ai::VTB>(
              vtb, true, dummy);
        }
      }
      break;
    }
  }

  return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  // make sure input is at least of size 10
  if (Size < 10) {
    return 0;
  }

  FuzzedDataProvider p(Data, Size);
  srand(p.ConsumeIntegral<uint32_t>());
  FuzzState state;

  bool hasEnoughData = true;
  do {
    try {
      hasEnoughData = handle(p, state);
    } catch (const NotEnoughDataException& e) {
      break;
    }
  } while (hasEnoughData);

  return 0;
}
