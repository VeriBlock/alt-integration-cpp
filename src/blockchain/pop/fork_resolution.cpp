#include <veriblock/blockchain/pop/fork_resolution.hpp>
#include <veriblock/consts.hpp>
#include <veriblock/keystone_util.hpp>

namespace VeriBlock {

namespace {

bool publicationViolatesFinality(int pubToCheck, int base) {
  int diff = pubToCheck - base;
  return diff - BITCOIN_FINALITY_DELAY > 0;
}

int getConsensusScoreFromRelativeBlockStartingAtZero(int64_t relativeBlock) {
  if (relativeBlock < 0 ||
      (size_t)relativeBlock >=
          POP_CONSENSUS_WEIGHTS_BY_RELATIVE_BITCOIN_INDEX.size()) {
    return 0;
  }

  return POP_CONSENSUS_WEIGHTS_BY_RELATIVE_BITCOIN_INDEX[relativeBlock];
}

struct KeystoneContextList {
  const std::vector<KeystoneContext>& ctx;

  KeystoneContextList(const std::vector<KeystoneContext>& c) : ctx{c} {}

  bool empty() const { return ctx.empty(); }

  int firstKeystone() const { return ctx[0].vbkBlockHeight; }

  int lastKeystone() const { return ctx[ctx.size() - 1].vbkBlockHeight; }

  const KeystoneContext* getKeystone(int blockNumber) const {
    if (!isKeystone(blockNumber, VBK_KEYSTONE_INTERVAL)) {
      throw std::invalid_argument(
          "getKeystone can not be called with a non-keystone block number");
    }

    if (blockNumber < this->firstKeystone()) {
      return nullptr;
    }

    if (blockNumber > this->lastKeystone()) {
      return nullptr;
    }

    auto i = (blockNumber - firstKeystone()) / VBK_KEYSTONE_INTERVAL;
    return &ctx.at(i);
  }
};

}  // namespace

int comparePopScore(const std::vector<KeystoneContext>& chainA,
                    const std::vector<KeystoneContext>& chainB) {
  KeystoneContextList a(chainA);
  KeystoneContextList b(chainB);

  if (a.empty() && b.empty()) {
    return 0;
  }

  if (a.empty()) {
    // a empty, b is not
    return -1;
  }

  if (b.empty()) {
    // b is empty, a is not
    return 1;
  }

  int earliestKeystone = a.firstKeystone();
  if (earliestKeystone != b.firstKeystone()) {
    throw std::invalid_argument(
        "comparePopScore can not be called on two keystone lists that don't "
        "start at the same keystone index");
  }

  int AlatestKeystone = a.lastKeystone();
  int BlatestKeystone = b.lastKeystone();
  int latestKeystone =
      BlatestKeystone > AlatestKeystone ? BlatestKeystone : AlatestKeystone;

  bool aOutsideFinality = false;
  bool bOutsideFinality = false;
  int chainAscore = 0;
  int chainBscore = 0;
  for (int keystoneToCompare = earliestKeystone;
       keystoneToCompare <= latestKeystone;
       keystoneToCompare += VBK_KEYSTONE_INTERVAL) {
    auto* actx = a.getKeystone(keystoneToCompare);
    auto* bctx = b.getKeystone(keystoneToCompare);

    if (aOutsideFinality) {
      actx = nullptr;
    }

    if (bOutsideFinality) {
      bctx = nullptr;
    }

    if (actx == nullptr && bctx == nullptr) {
      break;
    }

    if (actx == nullptr) {
      chainBscore += POP_CONSENSUS_WEIGHTS_BY_RELATIVE_BITCOIN_INDEX[0];
      // Nothing added to chainA; it doesn't have an endorsed keystone at
      // this height (or any additional height) Optimization note: if chainB
      // score is greater than A here, we can exit early as A will not have
      // any additional points

      continue;
    }

    if (bctx == nullptr) {
      chainAscore += POP_CONSENSUS_WEIGHTS_BY_RELATIVE_BITCOIN_INDEX[0];
      // Optimization note: if chainA score is greater than B here, we can
      // exit early as B will not have any additional points
      continue;
    }

    int earliestPublicationA = actx->firstBtcBlockPublicationHeight;
    int earliestPublicationB = bctx->firstBtcBlockPublicationHeight;

    int earliestPublicationOfEither =
        std::min(earliestPublicationA, earliestPublicationB);

    chainAscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationA - earliestPublicationOfEither);
    chainBscore += getConsensusScoreFromRelativeBlockStartingAtZero(
        earliestPublicationB - earliestPublicationOfEither);

    if (publicationViolatesFinality(earliestPublicationA,
                                    earliestPublicationB)) {
      aOutsideFinality = true;
    }

    if (publicationViolatesFinality(earliestPublicationB,
                                    earliestPublicationA)) {
      bOutsideFinality = true;
    }

    if (aOutsideFinality && bOutsideFinality) {
      break;
    }
  }

  return chainAscore - chainBscore;
}

}  // namespace VeriBlock
