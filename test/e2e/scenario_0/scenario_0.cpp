#include <gtest/gtest.h>

#include <string>
#include <util/pop_test_fixture.hpp>
#include <vector>
#include <veriblock/altintegration.hpp>
#include <veriblock/config.hpp>
#include <veriblock/mock_miner.hpp>

using namespace altintegration;

namespace generated {
extern const char atv[];
extern const char vtbs[];
extern const char btcbootstraps[];
extern const char vbkbootstraps[];
}  // namespace generated

static std::string dummy(std::string s) { return s; }

template <typename T>
static T fromHex(const std::string& h) {
  return T::fromHex(h);
}

template <typename T>
static std::vector<T> parse(const std::string& in,
                            std::function<T(std::string)> transform = dummy) {
  std::vector<T> ret;
  std::istringstream s(in);
  std::string line;
  while (std::getline(s, line)) {
    if (ParseHex(line).empty()) {
      break;
    }
    ret.push_back(transform(line));
  }
  return ret;
}

struct Scenario0 : public ::testing::Test {
  const int btcStart = 1717746;
  const int vbkStart = 438198;
  const int chainId = 3860170;
  std::shared_ptr<Config> config;
  std::shared_ptr<AltTree> alt;

  ATV atv;
  std::vector<VTB> vtbs;

  BtcBlock endorsedBlock = BtcBlock::fromHex(
      "00000020d9f80b499e1b51117a231981efbdb1b7237034a142526977614e4ccf06000000"
      "283ce1250d90fa23e476060c5e212813592f5c82066793807e73b15bc8fd4e82e4da995e"
      "ffff7f1d4b107a00");

  Scenario0() {
    config = std::make_shared<Config>();
    config->alt = std::make_shared<AltChainParamsRegTest>(chainId);

    config->btc.params = std::make_shared<BtcChainParamsTest>();
    config->btc.startHeight = btcStart;
    config->btc.blocks =
        parse<BtcBlock>(generated::btcbootstraps, fromHex<BtcBlock>);

    config->vbk.params = std::make_shared<VbkChainParamsTest>();
    config->vbk.startHeight = vbkStart;
    config->vbk.blocks =
        parse<VbkBlock>(generated::vbkbootstraps, fromHex<VbkBlock>);

    auto t = Altintegration::create(*config);
    alt = std::make_shared<AltTree>(std::move(t));

    atv = parse<ATV>(generated::atv, fromHex<ATV>)[0];
    vtbs = parse<VTB>(generated::vtbs, fromHex<VTB>);
  }
};

TEST_F(Scenario0, Scenario0) {
  ASSERT_NO_THROW(config->validate());

  AltBlock endorsedPrev;
  endorsedPrev.hash = endorsedBlock.previousBlock.asVector();
  endorsedPrev.height = 1;
  endorsedPrev.timestamp = 10000;
  endorsedPrev.previousBlock = config->alt->getBootstrapBlock().hash;

  AltBlock endorsed;
  endorsed.hash = endorsedBlock.getHash().asVector();
  endorsed.previousBlock = endorsedPrev.hash;
  endorsed.timestamp = 10001;
  endorsed.height = 2;

  AltBlock containing;
  containing.height = 3;
  containing.previousBlock = endorsed.hash;
  containing.timestamp = 10002;
  containing.hash = std::vector<uint8_t>{1, 3, 3, 10};

  AltPayloads payloads;
  payloads.hasAtv = true;
  payloads.atv = atv;
  payloads.vtbs = vtbs;
  payloads.containingBlock = containing;
  payloads.containingTx = uint256();
  payloads.endorsed = endorsed;

  ValidationState state;
  ASSERT_TRUE(alt->acceptBlock(endorsedPrev, state)) << state.toString();
  ASSERT_TRUE(alt->acceptBlock(endorsed, state)) << state.toString();
  ASSERT_TRUE(alt->acceptBlock(containing, state)) << state.toString();
  ASSERT_FALSE(alt->addPayloads(containing, {payloads}, state));
  ASSERT_EQ(
      "0+bad-alt-payloads-stateful+0+apply-context+0+alt-accept-block+0+bad-prev-block",
      state.GetPath());
}