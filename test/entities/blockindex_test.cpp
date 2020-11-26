// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include <gtest/gtest.h>

#include <string>
#include <veriblock/blockchain/block_index.hpp>
#include <veriblock/entities/altblock.hpp>
#include <veriblock/entities/btcblock.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/serde.hpp>

static const std::string AltBlockIndexVbkEncoded =
    "00000016000000207b7d4979c91f10b52a23b3c477582cc855be035dde1552017383a4a152"
    "9155cd0000002085ac9095fe4cd696eb0b13fe9953a46851fd913b072012f5225d31a32ece"
    "edb30000001600000016000000c80100010120b8eada970e8e6a1709246688df98c142c2bb"
    "f654f82e53c2d6ebc31890e383d1010120c5a370443608ed9a068464e9ddb553ca79c05891"
    "283539fe60de75b4638e03ff01440cfbd58d6e25fa183dff8c499b0cb8350c1e2317d0df60"
    "75f7130c3de9e7ffa84eb01ae5fcda1c0c0fda42b0553ae350acf509e80c88dce1ae2392c8"
    "012b0a1e880ceb4a8b1a67dd12c193c459f30c3e7c09210907ede7910391f20c1bfc65abf8"
    "76c25c31ca5b560ceffc1c57d659f9fb774491240c95c570d09968d57c60bb6ab00c620c15"
    "b7a9ca2d73e2b1eb0f0c621b844b52561859a1a08f940c1c54d530ac72d3f60095c51f0c83"
    "bc8984eb020cf594e512f60c6b81cf5a354a54833045934a0c6a7a7eefa0013f450858d062"
    "0caee5c649be69ca179a4f9c1e0c197ec90a078e307bb43bdcbd0ce954495ce27295feef40"
    "c14a0cca4db49b54beeb0e975116040c5f3321448da21d559b4ed9170c98b64d822925394b"
    "640cf79a0cb48420f24645c71ad74d18420cf89f4b83eebaf67e8e0749d30c53b6119d21c1"
    "c938414601620c0bc41e4ecd251564f53439150cf52c730434dd429b94200cb10c988d5bc9"
    "7995698ac66dc5d60c001ff8730a3912b947d1345b0c5812967cbf1fe63d1fe1cf350cd432"
    "67647cc34a2588c66e100c0b526a50a6d144ea3ce503e00cd81d5ab87e6221cf6f4004fb0c"
    "f947f8c3b54a0db0c75eb0dd0c4d341e7048e911fa137b0fad0c6dc7dad2d879d9987a2be2"
    "ae0c386077206c9793a02417fec30c299723dd02e452c5de1070b10c3329f027216d6b6326"
    "d55d1d0cda0c0f30fbe25f14df653a9e0c3d77a0977c3c560eb56808a00cd11ae3f002080b"
    "38a04091fa0cce7ff2a4041729069c7ee9ff0c439f68649a308ed36797df2e0cc74b692d61"
    "68c3db0048babd0c5301f51bfc9da035e2d722280cbdce322405a4e45730f098460ca3dc82"
    "c96e6fe7dca71941ea0cc7c3b21bae74f256948cae140caf08ee25e6d389ff894635590cd5"
    "d5ffe26af15bb2b0c852000cbb266b2db2908c7324c729a50cd6b41e1451d135b51a518501"
    "0c16c7e81eddb45e70974e2aca0ceca56b4743a8b96060ebb3d20cba4c980c0c8ec126530b"
    "a7a90cb26ddd6b02087a81d73b6c300c2c41fe3830c7832a8e1b7e700cbcff0bccf4ccfc82"
    "16a797b30c7ed2c2f068644a61959238850cef773a2cc892e575bc6795f10c8066c7757e51"
    "1321c42900280c03e00f0d69d4c42cc3d285fb0c4311f586620790913a0d26270ce0975a0b"
    "5b0fc631e49cccb60cdd5179ee90f2d59ab819ab1c0c155cee0bcb2e82ee6120d9110c061e"
    "55dad51364ca7651348f";

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
