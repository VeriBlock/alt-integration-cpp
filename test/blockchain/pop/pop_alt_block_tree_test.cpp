#include <gtest/gtest.h>

#include <exception>
#include <memory>

#include "util/pop_test_fixture.hpp"
#include "veriblock/blockchain/pop/fork_resolution.hpp"

using namespace altintegration;

struct AltTreeTestFixture : public ::testing::Test {
  MockMiner popminer;
};

TEST_F(AltTreeTestFixture, basic_test) {}
