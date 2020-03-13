#include <gtest/gtest.h>

#include "veriblock/blockchain/pop/fork_resolution.hpp"
#include "veriblock/blockchain/vbk_chain_params.hpp"

using namespace VeriBlock;

const static VbkChainParamsRegTest param;

struct TestCase {
  using V = std::vector<KeystoneContext>;
  V A;
  V B;
};

struct PopForkResolutionEqual : public ::testing::TestWithParam<TestCase> {};
struct PopForkResolutionAwins : public ::testing::TestWithParam<TestCase> {};

TEST_P(PopForkResolutionEqual, Equal) {
  auto [A, B] = GetParam();
  ComparePopScore comparePopScore(param.getKeystoneInterval());
  ASSERT_EQ(comparePopScore(A, B, param), 0);
  ASSERT_EQ(comparePopScore(B, A, param), 0);
}

TEST_P(PopForkResolutionAwins, Awins) {
  auto [A, B] = GetParam();
  ComparePopScore comparePopScore(param.getKeystoneInterval());
  ASSERT_GT(comparePopScore(A, B, param), 0);
  ASSERT_LT(comparePopScore(B, A, param), 0);
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
    // clang-format on
};

INSTANTIATE_TEST_SUITE_P(Pop,
                         PopForkResolutionEqual,
                         ::testing::ValuesIn(EqualCases));

INSTANTIATE_TEST_SUITE_P(Pop,
                         PopForkResolutionAwins,
                         ::testing::ValuesIn(AwinsCases));
