#include <gtest/gtest.h>

#include <veriblock/blockchain/btc_chain_params.hpp>
#include <veriblock/blockchain/vbk_chain_params.hpp>
#include <veriblock/stateless_validation.hpp>

using namespace altintegration;

template <typename T>
struct ParamsTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(ParamsTest);

TYPED_TEST_P(ParamsTest, GetGenesisBlock) {
  TypeParam param;
  ASSERT_TRUE(checkProofOfWork(param.getGenesisBlock(), param))
      << "bad POW of genesis block";
}

REGISTER_TYPED_TEST_SUITE_P(ParamsTest, GetGenesisBlock);

// clang-format off
typedef ::testing::Types<
    VbkChainParamsMain,
    VbkChainParamsTest,
    VbkChainParamsAlpha,
    VbkChainParamsRegTest,
    BtcChainParamsMain,
    BtcChainParamsTest,
    BtcChainParamsRegTest
    > AllParams;
// clang-format on

INSTANTIATE_TYPED_TEST_SUITE_P(Chain, ParamsTest, AllParams);
