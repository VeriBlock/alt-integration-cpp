// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <string>
#include <util/test_utils.hpp>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/serde.hpp>

static const std::string AltBlockIndexVbkEncoded =
    "0000000b20f33544a7391f71e6ca8889d1a058aabf76597f7bc481142303bac42530a7e223"
    "20954db438edd1e31e5387192fe18c9c2bfcad9fac23699fcedec4ea8cccd515620000000b"
    "0000000b00000160010220ff4b2bb098832cee9012c2f2a2e7e19739671b03196e7c2bfeb2"
    "ccc32d37ca8520f4d19d5cd16e5db73260621837d87936b2c896bfb55c9c83eacdedff663c"
    "315a20f33544a7391f71e6ca8889d1a058aabf76597f7bc481142303bac42530a7e22318fe"
    "62c7297188f4bcbf77ca1777fab998b52075256b16d39d0a0102030405060708090a208e7e"
    "f17fbf4f8491ee7d31f9492be0f564e840d1c5150f39977f45ce3195b6e720f4d19d5cd16e"
    "5db73260621837d87936b2c896bfb55c9c83eacdedff663c315a20f33544a7391f71e6ca88"
    "89d1a058aabf76597f7bc481142303bac42530a7e2231833dd49499e43de8da4c9b11190c8"
    "06f6556ec8aae7a67b9f0a0102030405060708090a0102208e7ef17fbf4f8491ee7d31f949"
    "2be0f564e840d1c5150f39977f45ce3195b6e720ff4b2bb098832cee9012c2f2a2e7e19739"
    "671b03196e7c2bfeb2ccc32d37ca8501022087c0eebb4d87807651bda3d342514b1eeb1109"
    "afb50afd98fb2e40347bff9d352075af5036ff145a7e67c9045da90eb157f321f7148de37a"
    "8965e95beb3de51ab101b10ce1f7df9d006885f2ed009f8c0cc5fcfb31a7a1b798753cfee1"
    "0c1bc9711cb789f21853f8d5fc0c2d07d5fe49058ae768b7e4190c58c901d5a48f92bb1bac"
    "c3ae0c9e077ad12b5e4fde48416bca0cc2573b1e07378eace3d5219f0c094b4378181da352"
    "0897ad4a0c100fad5846f4d6310f5433030c41a047a66e8ff4b46dd555a20c4f39981ddf8d"
    "dc454e5cb2120cd1bc0cec99598a413f6a44750c3e195b84ff821471918495890c7b1b51eb"
    "76b285e17fdbbe480c4aebe7e439345cbb030d54f40c0ffe8bbc41de67b6119b393c0ce683"
    "5247047b43721d032a450c8c69f3e4e6d6153dd438fb660ce8ddb689d933e72b8fb59d7a0c"
    "168c3007f44ddafdcb63ff210c270a016e2a8b53fd474a46720ccdc9a18b8c37075caa81e6"
    "5c0c80b5cde420d74bf7f7dbef790c9eb17936a979bc34d8f1a4cf0c7e2ccae9f3cd9594b5"
    "ec30070c6bcbcc3ca1e46b942592928c0c777e97f9e7756cc81962b3af0c796fe502779b98"
    "aa0204bb020c73777044dd94b6fa245213bd0cb6d6fffa242f31578abf4e420cd4efac28f1"
    "2a383bdef5f52d0cbce7b522b92006055288b9880c6c8641229c15bb6c40080e630c67b8d0"
    "65a0d80b23492f7e770cf7684286a361ad37ca2d72b10c66ead710ff54e0b248d534690cd8"
    "3303babdbf2309a8828f070cb5797a4fc367346ab10ec3740ca61f35e9e9ba8fe1129997fe"
    "0c551d2aa767f180c4c118958b0c90bbc736ba5f3682c54b22a90cf8bbc783e5d7e104db82"
    "875a0c0526d4aa98a54c70dd9e58ae0c34c16c196f1a3ff7c90091000c953ab89d6f55871b"
    "d4d37ea60ce172a1820f98eceed03974300c8c384c889bb2610d9d6c4de10cb813a279ebf5"
    "e50f0a1f169a0c63b67b38269ff18e42c29e530cf6fa29a6970777c74ade42950c8132dc2b"
    "aa332139d470ad610c3b70e554a3bee63b611d41ed0c3c18083ac5bc37618c97355a0cd74e"
    "975a1b33cbd961779b4b0ce61f31a92d1312c921d316570c00854a196b561ac13415c2df0c"
    "b9c9c0cebad80799325fe2950c347b8c9103037cf25e86e59e0cc8a27af6fb93b40097b2d3"
    "580c6412c502508210a3c4e2cf370c57a383d6511708604de3d0e60c9b21f6ee1481391116"
    "4817080c4b94b0496f47d4d7dbdd7b3f0c9b841763fed7b81511a594250ce0d1515db2a439"
    "89ba9dd01d0cd0da61b1acf23b7f17f115f00ce5e9b60054251fd567ff62d60c5d8d2f2fad"
    "4a9da69bf5477c0c38a11d5010203383989aecbf0c7369b3ce5653b3ed032ea7130c018d46"
    "d72f573a2a452453690c76f7f636d1a4b437bb46af9d0c5134f373f9c0703fe730e25d0c91"
    "8d1d07e2f5affcfeb98a3f0c5f090b31a47292f87f2601020c9a455e963794a9085f8fcba0"
    "0c3547ccc625ceb4877bcc9d2a0c14b7dc40ebd7be5ab71284f70c26711e267471be65cb8f"
    "8ee70c33effe020126d9238076e8ea0c12c68fdd55d2a7720ac706160c96ed15a6e2c4bf5f"
    "c9f0bf3f0c6a00847999a127fcf65bf27a0ca349f0473347f5ebc8116e380c4a7b3e068daf"
    "c3a7299721df0c90be4661fa2aa1cfa69b94c10ccadd9a28d641e7e54b8a4c0d0c970ad0c7"
    "6ffde5f689f26f740ce35e3d736060b132b91c53410c5c05590300fe08dedc8cdc3a0c249e"
    "2f1f21f0dcf7b9f5e7720ca3b275dc16b73ab3a51139e10cd71138292abe4b46215199b30c"
    "b455100856151d95a71b01db0c000f675bdd49245db883c9470c14ef92b93400d160195b7e"
    "bc0c2d9e1cd8fcb4fe4425e194350c31cd3bee81336a8460fc701b0c0c996c150694f010e7"
    "9a00630c3a733766c115eba2c14c31270ce41b6fc2df143d8ebe8924f50ce1c3ef725f3590"
    "0a6fb4df680c914527ec8f99988c58db59b50ce470213a74d9bf786a313c3b0cc66d3667e9"
    "0fd538e590af940cfb3e8c9447cb4327aeb554960c1339769f1b3ab84b5c07babd0c04bd92"
    "49ef48abda013212710c1058114ee781476f30153c870cf5720eb3914e43f44da9cd730cda"
    "08d866299d88609357807d0c607e2190f2b9a93b882b33ff0c7c7df87da69f9cde5b24ae87"
    "0c343a620406bbb8c71e23536a0cbe2a8ed83a7d939c81ac67240c04165e2796372894054e"
    "f0740c4e6ef95ad74529b66837ef790c00e696707bbd61ce51469e830c313ec49a2fd6009f"
    "39cde7340caf1558aac10a2cc8de182b0c0c90c806f6556ec8aae7a67b9f0ce8ec0c148d94"
    "d9c2cf6c57130cddbfa9c42ad983ea9228a2150cfef9f6fb7a432148f35949eb0cba3da9d3"
    "6c2ee97f14967ca40c8d576353e629da13224acf700c6eacbe58e2d7a82da54bcd0c0c5b12"
    "93420ddca97ae35629ba0c499bd650836b3714e550173b0c4879ca38375e29bd6cb15b610c"
    "eb631f11ecc94025654245d30cdab0bd8388f5780b7681fd1c0cce8649df5b536457a8fd62"
    "3b0c2cf5dd129f33aab230fb66350c0355b041d34426579f6d6cdf0cc7475f63472ceb8c30"
    "6476480c04649597806078925eb238f70ccc2e6d34a8221eb558f060bf0c9c7f894f909506"
    "08d3c970e40c43f8856d22eb917608ae93c10c141765e651d41d3321387ce30ccc12d2e267"
    "2003a18a3263340c5e048b8cc185ab51a506b38b0c3b8be41b0f55a2a3c6e8231f0c6339a3"
    "7df7b6bf43961220c00c32a0de4ab782a02f10406e210c46604bdd5ee9b75bb411756f0c14"
    "8dd0aed3e01c0416a45ba90c5c39a8a08970cae1e01612140cfc48265850caee4c90f8631e"
    "0c104098fa942c72e6bf4877c90c0eca21e6407b73e3d5a35b360c01f0c4e806972ed86ad1"
    "4c5b0c55eb3472d3e5939f2b4be7a80ca84a66c27ca40d6bbb94b9ec0ca9c42fac7f3f2d46"
    "c632d8540cb6acfe5812f28d6bdcad26550c6dfe3ed16543fde3fbf2390b0cf902e5346698"
    "96c27f6a32400cb4aa8d6d364a7178b9554f0c0c73a0ec75bb7071168be11b650c0cecb0d2"
    "4c618c2e9ab562280c7508a25b10fdddec28e7e5550cd4d5b84b3d7aa1d23af8d4b80cee98"
    "dc166014267af2f138b90caadb75d72ea1190da751480d0c0e20b89e85fe958d408ebc840c"
    "0f0d42c5bb6c6bf1e4888a8d0c51ae2db1fcff6a8f0d46334c0c3646679a78ca9557617b37"
    "340c60f254543d2814429bd924d20c33e5fb95adab23dfbd0fb5ee0cdcf64eeeeba67012d9"
    "5dc9710ce953b19d9e5ae3e3bb9916e00c0c15dcadddcf07cd3dfc1f8e0c291779d36b19b8"
    "2d137adc490c77fab998b52075256b16d39d";

static const std::string VbkBlockIndexVbkEncoded =
    "00000043000000430002dd5179ee90f2d59ab819ab1cf068644a619592388530fbe25f14df"
    "653a9ead29e41ee6c8d135c0a3c845b180dde35f8c848b0101000000000000000000016000"
    "00000101012084c5ef4592fdc67038e69ebfa32a52ff6c81baa82301744281a270dfab8f82"
    "fb1850e9e995583c09170c7d9510ba4c980c0c8ec126530ba7a918b2323cec803c693d0596"
    "370e155cee0bcb2e82ee6120d91120166d85b68da59f567cbed28a664f5ce96592d953e0ce"
    "fed8f8e14acb63fd17190001012084c5ef4592fdc67038e69ebfa32a52ff6c81baa8230174"
    "4281a270dfab8f82fb";

static const std::string BtcBlockIndexVbkEncoded =
    "000000010100000006226e46111a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf1"
    "88910fa1dba9aa0a3ef1eadb813f49d8910589eda0d01213d0e469da78301da0f72d8a8b84"
    "8c5fffff7f2000000000000000c0010100000043";

using namespace altintegration;

TEST(BlockIndex, BTC) {
  BlockIndex<BtcBlock> index;
  auto data = ParseHex(BtcBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, VBK) {
  BlockIndex<VbkBlock> index;
  auto data = ParseHex(VbkBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state));
}

TEST(BlockIndex, ALT) {
  BlockIndex<AltBlock> index;
  auto data = ParseHex(AltBlockIndexVbkEncoded);
  ReadStream stream(data);
  ValidationState state;
  ASSERT_TRUE(DeserializeFromVbkEncoding(stream, index, state))
      << state.toString();
}

template <typename TestCase>
struct BlockIndexTest : public ::testing::Test {};

TYPED_TEST_SUITE_P(BlockIndexTest);

TYPED_TEST_P(BlockIndexTest, RoundTrip) {
  using Block = TypeParam;
  using Index = BlockIndex<Block>;
  Index index = getRandomIndex<Block>();
  ValidationState state;

  auto vbkencoded = SerializeToVbkEncoding<Index>(index);
  Index decodedVbk;
  ASSERT_TRUE(DeserializeFromVbkEncoding<Index>(vbkencoded, decodedVbk, state));
  ASSERT_TRUE(state.IsValid());
  ASSERT_EQ(index.toVbkEncoding(), decodedVbk.toVbkEncoding());
}

// clang-format off
typedef ::testing::Types<
    BtcBlock,
    VbkBlock,
    AltBlock
> TypesUnderTest;
// clang-format on

REGISTER_TYPED_TEST_SUITE_P(BlockIndexTest, RoundTrip);

INSTANTIATE_TYPED_TEST_SUITE_P(BlockIndexTestSuite,
                               BlockIndexTest,
                               TypesUnderTest);
