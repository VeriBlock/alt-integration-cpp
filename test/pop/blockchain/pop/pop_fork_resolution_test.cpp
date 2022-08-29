// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/pop/fork_resolution.hpp>
#include <veriblock/pop/blockchain/vbk_chain_params.hpp>

using namespace altintegration;

const static VbkChainParamsRegTest param;

struct MockReducedPublicationView
    : private std::vector<internal::KeystoneContext> {
  using base_t = std::vector<internal::KeystoneContext>;

  MockReducedPublicationView(
      std::initializer_list<internal::KeystoneContext> il)
      : vector(il) {}

  using base_t::empty;
  using base_t::size;
  int firstKeystone() const { return (*this)[0].blockHeight; }

  int lastKeystone() const { return (*this)[size() - 1].blockHeight; }

  const VbkChainParamsRegTest& getConfig() const { return param; }
  int nextKeystoneAfter(int keystoneHeight) const {
    return keystoneHeight + getConfig().getKeystoneInterval();
  }

  const internal::KeystoneContext* getKeystone(int blockHeight) {
    if (blockHeight < firstKeystone() || blockHeight > lastKeystone()) {
      return nullptr;
    }

    auto it = std::find_if(
        begin(), end(), [blockHeight](const internal::KeystoneContext& kc) {
          return kc.blockHeight == blockHeight;
        });

    return it == end() ? nullptr : &*it;
  }
};

struct TestCase {
  using V = std::vector<internal::KeystoneContext>;
  MockReducedPublicationView A;
  MockReducedPublicationView B;
};

struct PopForkResolutionEqual : public ::testing::TestWithParam<TestCase> {};
struct PopForkResolutionAwins : public ::testing::TestWithParam<TestCase> {};

TEST_P(PopForkResolutionEqual, Equal) {
  auto [A, B] = GetParam();
  ASSERT_EQ(internal::comparePopScoreImpl(A, B), 0);
  ASSERT_EQ(internal::comparePopScoreImpl(B, A), 0);
}

TEST_P(PopForkResolutionAwins, Awins) {
  auto [A, B] = GetParam();
  ASSERT_GT(internal::comparePopScoreImpl(A, B), 0);
  ASSERT_LT(internal::comparePopScoreImpl(B, A), 0);
}

static const std::vector<TestCase> EqualCases = {
    // clang-format off
    /**
     * Scenario: two chains with 7 identical keystone contexts
     *
     * Expected: Chains should be equal to one another
     */
    {
      {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}},
      {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}}
    },
    /**
     * Scenario: two chains with 6 identical keystone contexts, and 1 keystone has a difference of 1 BTC block
     *
     * Expected: Chains should be equal to one another
     */
    {
      {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}},
      {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 6}}
    },
    /**
     * Scenario: Chain B is one BTC block after chain A for all keystones
     *
     * Expected: Chains should be equal to one another
     */
    {
      {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}},
      {{10000, 2},{10020, 2},{10040, 3},{10060, 4},{10080, 4},{10100, 5},{10120, 6}}
    },
    /**
     * Scenario: two chains with 1 identical keystone context
     *
     * Expected: Chains should be equal to one another
     */
    {
      {{10000, 100}},
      {{10000, 100}},
    },
    /**
     * Scenario: two chains with 5 identical keystone contexts, chain A has one keystone that's 2 worse,
     * chain B has one keystone that's 2 worse
     *
     * Expected: Chains should be equal to one another
     */
    {
        {{10000, 1},{10020, 3},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}},
        {{10000, 1},{10020, 1},{10040, 4},{10060, 3},{10080, 3},{10100, 4},{10120, 5}}
    },
    /**
     * Scenario: two chains with 3 identical keystone contexts, chain A has one keystone that's 2 worse, one that's
     * 3 worse, and chain B has one keystone that's 2 worse, one that's 3 worse
     *
     * Expected: Chains should be equal to one another
     */
    {
        {{10000, 1},{10020, 2},{10040, 3},{10060, 3},{10080, 5},{10100, 7},{10120, 5}},
        {{10000, 1},{10020, 2},{10040, 3},{10060, 6},{10080, 3},{10100, 4},{10120, 7}}
    },
    /**
     * Scenario: two chains with 10 identical keystone contexts
     *
     * Expected: Chains should be equal to one another
     */
    {
        {{2000, 1},{2020, 2},{2040, 3},{2060, 4},{2080, 5},{2100, 6},{2120, 7}, {2140, 8}, {2160, 9}, {2180, 10}},
        {{2000, 1},{2020, 2},{2040, 3},{2060, 4},{2080, 5},{2100, 6},{2120, 7}, {2140, 8}, {2160, 9}, {2180, 10}}
    },
    /**
     * Scenario: Chain A and Chain B both have an equal-sized gap, so the gap should be ignored on both chains.
     *           The keystone gap also includes a BTC finality delay violation gap, which should be ignored because of the keystone gap.
     *
     * Expected: Chains should be equal to one another
     */
    {
        // gap of keystones 60 and 80
        {{20, 98},{40, 100},{100, 120},{120, 122}},
        // gap of keystones 60 and 80
        {{20, 98},{40, 100},{100, 121},{120, 121}}
    },
};
// clang-format on

static const std::vector<TestCase> AwinsCases = {
    // clang-format off
    /**
     * Scenario: two chains with 6 identical keystone contexts, and 1 keystone has a difference of 2 BTC block
     *
     * Expected: Chain A should be better than Chain B
     */
    {
        {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 5}},
        {{10000, 1},{10020, 1},{10040, 2},{10060, 3},{10080, 3},{10100, 4},{10120, 7}}
    },
    /**
     * Scenario: two chains with 3 identical keystone contexts, chain A has one keystone that's 2 worse, one that's
     * 3 worse, and chain B has one keystone that's 2 worse, one that's 4 worse
     *
     * Expected: Chains A should be better than Chain B
     */
    {
        {{10000, 1},{10020, 2},{10040, 3},{10060, 3},{10080, 5},{10100, 7},{10120, 5}},
        {{10000, 1},{10020, 2},{10040, 3},{10060, 7},{10080, 3},{10100, 4},{10120, 7}}
    },
    /**
     * Scenario: two chains with 3 identical keystone contexts, chain B has one keystone that's 2 worse, one that's
     * 4 worse, and chain A has two keystones that are 3 worse
     *
     * Expected: Chains A should be better than Chain B
     */
    {
        {{10000, 1},{10020, 2},{10040, 3},{10060, 3},{10080, 7},{10100, 8},{10120, 6}},
        {{10000, 1},{10020, 4},{10040, 7},{10060, 3},{10080, 4},{10100, 5},{10120, 6}}
    },
    /**
     * Scenario: Chain B is ahead of chain A by two blocks until the end, where it hasn't received a publication yet
     *
     * Expected: Chain A should be better than Chain B
     */
    {
        {{2000, 3},{2020, 4},{2040, 5},{2060, 6},{2080, 7},{2100, 8},{2120, 9}, {2140, 10}, {2160, 11}, {2180, 12}},
        {{2000, 1},{2020, 2},{2040, 3},{2060, 4},{2080, 5},{2100, 6},{2120, 7}, {2140, 8}, {2160, 9}}
    },
     /**
     * VeriBlock chain B wins because chain A has a publication gap; keystone A40 was published to Bitcoin block 100 and keystone B40 was published to Bitcoin block 102,
     * so when the next keystones (A60 and B60) in both chains were published to Bitcoin block 112, VeriBlock chain A has a context gap (>11 BTC blocks between publications of two keystones),
     * but B60 has a context gap <11, so PoP score for 2nd different keystone period (A60 vs B60) is A=0 B=1, making chain B have a higher PoP score.
     * Maxwell Sanchez comments.
     * Expected: Chain B should be better than Chain A
     */
    {
        {{20, 98},{40, 102},{60, 112}}, // B chain
        {{20, 98},{40, 100},{60, 112}} // A chain
    },
     /**
     * Scenario: Chain A has two publication gaps, B has one publication gap
     * Expected: Chain B should be better than Chain A
     */
    {
        {{20, 98},{40, 102},{60, 112}, {80, 117}, {100, 129}}, // B chain
        {{20, 98},{40, 100},{60, 112}, {80, 117}, {100, 129}} // A chain
    },
    /**
     * Scenario: even though chain B started ahead with publication of B60 to earlier BTC block,
     *           A80 had better PoP publication which made its total score better
     * Expected: chain A should be better then chain B
     */
    {
        {{20, 98}, {40, 100}, {60, 103}, {80, 111}},
        {{20, 98}, {40, 100}, {60, 102}, {80, 114}},
    },
    /**
     * Scenario: Chain A and Chain B both have a gap, but Chain B's gap is larger.
     *           Chain B should be chopped starting at it's gap, and A's gap should be retained.
     * Expected: chain A should be better then chain B
     */
    {
        // gap of keystone 60
        {{20, 98}, {40, 100}, {80, 110}, {100, 111}, {120, 112}},
        // gap of keystones 60 and 80
        {{20, 98}, {40, 100}, {100, 101}, {120, 102}},
    },
    /**
     * Scenario: Chain A and Chain B both have a gap of the same size, but Chain B has a Bitcoin-finality-delay-violating gap before it.
     *           Chain B should be chopped starting at it's Bitcoin finality delay violation
     * Expected: chain A should be better then chain B
     */
    {
        // gap of keystones 60, 80
        {{20, 98}, {40, 100}, {100, 111}, {120, 112}},
        // gap of keystones 60 and 80
        {{20, 98}, {40, 112}, {100, 101}, {120, 102}},
    },
    /**
     * Scenario: Chain A and Chain B both have a gap of the same size, and then a second gap where B's gap starts before A's.
     *           Chain B should be chopped starting at the beginning of its second gap.
     * Expected: chain A should be better then chain B
     */
    {
        // gap of keystones 60 and 80
        {{20, 98}, {40, 100}, {100, 111}, {120, 117}, {140, 120}, {180, 124}},
        // gap of keystones 60, 80, 140, 160
        {{20, 98}, {40, 100}, {100, 111}, {120, 112}, {180, 119}}
    },
    /**
     * Scenario: Chain A and Chain B both have a gap of the same size, and then a second gap where A's gap starts before B's.
     *           Chain A should be chopped starting at the beginning of its second gap. However, chain A has a much higher score, so even
     *           the rounds where Chain B has a keystone and Chain A doesn't, Chain A still wins in the end because of it's early lead.
     * Expected: chain A should be better then chain B
     */
    {
        // gap of keystones 60, 80, 140, 160
        {{20, 98}, {40, 100}, {100, 111}, {120, 112}, {180, 119}},
        // gap of keystones 60, 80, 160
        {{20, 107}, {40, 110}, {100, 116}, {120, 120}, {140, 122}, {180, 124}}
    },
    /**
     * Scenario: Chain A and Chain B both have a gap of the same size, and then a second gap where B's gap starts before A's.
     *           Chain B should be chopped starting at the beginning of its second gap. Chain B has a much higher score in the beginning, but the chop
     *           allows Chain A to eventually surpass it (even though Chain B has earlier publications than A even in the tip, Chain B was chopped).
     * Expected: chain A should be better then chain B
     */
    {
        // gap of keystones 60, 80, 160
        {{20, 107}, {40, 110}, {100, 116}, {120, 120}, {140, 122}, {180, 124}, {200, 129}, {220, 129}, {240, 130}},
        // gap of keystones 60, 80, 140, 160
        {{20, 98}, {40, 100}, {100, 111}, {120, 112}, {180, 119}, {200, 125}, {220, 126}, {240, 128}}
    },
    // clang-format on
};

INSTANTIATE_TEST_SUITE_P(Pop,
                         PopForkResolutionEqual,
                         ::testing::ValuesIn(EqualCases));

INSTANTIATE_TEST_SUITE_P(Pop,
                         PopForkResolutionAwins,
                         ::testing::ValuesIn(AwinsCases));
