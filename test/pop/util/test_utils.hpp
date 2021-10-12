// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_TEST_UTILS_HPP
#define ALT_INTEGRATION_TEST_UTILS_HPP

#include <stdint.h>

#include <algorithm>
#include <random>
#include <vector>
#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/entities/altblock.hpp>
#include <veriblock/pop/entities/popdata.hpp>

namespace altintegration {

static const std::string defaultAtvEncoded =
    "0000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e8000107"
    "01370100010c6865616465722062797465730112636f6e7465787420696e666f2062797465"
    "7301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c"
    "47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd5"
    "38b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b810400"
    "0a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6"
    "c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e0400000001"
    "0400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed2253"
    "1c040000000220000000000000000000000000000000000000000000000000000000000000"
    "00002000000000000000000000000000000000000000000000000000000000000000004100"
    "0013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bb"
    "fda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001";

static const std::string defaultVtbEncoded =
    "0000000102046102011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae264100001335"
    "0002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a156"
    "28b06ba3b44c0190b5c0495c9b8acd0701c523005ebbbe9c02011b01000000010ce74f1fb6"
    "94a001eebb1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a4730440220"
    "0cf4998aba1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca"
    "24dc520915b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c"
    "395a82ef0bd63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d0000"
    "0000001976a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000"
    "536a4c50000013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09ef"
    "ac4be99f95f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe186"
    "4df04216615cf92083f40000000002019f040000067b040000000c04000000040000002020"
    "4d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe"
    "8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af"
    "133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943"
    "af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f"
    "30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc"
    "31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc"
    "2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74"
    "453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17"
    "202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba22"
    "9acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1"
    "a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000"
    "000014297d038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d"
    "74a999c26acd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc00103500000"
    "80200000000000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eef"
    "bc6d8670ee98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17"
    "e11874af500000402000000000000000000013535112250e115e2896e4f602c353d8394430"
    "80398e3f1dfb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e0"
    "8d9b5c6c1f2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17d"
    "d3316a827f5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b64"
    "3406bed63aa310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e"
    "47e90a348efd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb00"
    "7a3b2a25c550f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a"
    "03420004b3c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8"
    "a754c06a22760e44c60642fba883967c19740d5231336326f7962750c8df99040000000004"
    "0000000d202a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec6"
    "0400000006205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997"
    "872020d0a3d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06f"
    "e913dca5dc2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350ee"
    "b8b3df630c8308b5c8c2ba4cd6210868395b084af84d19ff0e902000000000000000000000"
    "000000000000000000000000000000000000000000002036252dfc621de420fb083ad9d876"
    "7cba627eddeec64e421e9576cee21297dd0a41000013700002449c60619294546ad825af03"
    "b0935637860679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b92"
    "11070211cafff01c3f01";

static const std::string defaultVbkBlockEncoded =
    "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e"
    "26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001";

template <typename T>
void generateRandomBytes(T begin, T end) {
  std::generate(begin, end, []() -> uint8_t { return rand() & 0xFF; });
}

inline std::vector<uint8_t> generateRandomBytesVector(size_t n) {
  std::vector<uint8_t> bytes(n);
  generateRandomBytes(bytes.begin(), bytes.end());
  return bytes;
}

template <size_t N>
inline Blob<N> generateRandomBlob() {
  return generateRandomBytesVector(N);
}

template <typename B>
StoredBlockIndex<B> getRandomIndex();

template <>
inline StoredBlockIndex<BtcBlock> getRandomIndex() {
  StoredBlockIndex<BtcBlock> index;
  index.height = rand();

  BtcBlock block(
      /*version=*/rand(),
      /*prev=*/generateRandomBytesVector(32),
      /*mroot=*/generateRandomBytesVector(32),
      /*time=*/rand(),
      /*bits=*/rand(),
      /*nonce=*/rand());
  index.header = std::make_shared<BtcBlock>(block);
  index.addon.refs.push_back(rand());
  index.status = rand() & 0x0f;
  return index;
}

template <>
inline StoredBlockIndex<VbkBlock> getRandomIndex() {
  StoredBlockIndex<VbkBlock> index;
  index.height = rand();

  VbkBlock block;
  block.setVersion((int16_t)rand());
  block.setPreviousBlock(generateRandomBytesVector(uint96::size()));
  block.setPreviousKeystone(generateRandomBytesVector(uint72::size()));
  block.setSecondPreviousKeystone(generateRandomBytesVector(uint72::size()));
  block.setTimestamp(rand());
  block.setDifficulty(rand());
  block.setNonce(rand());
  block.setMerkleRoot(generateRandomBytesVector(uint128::size()));
  index.header = std::make_shared<VbkBlock>(block);
  index.status = rand() & 0x0f;
  index.addon._refCount = 100;
  index.addon._vtbids.push_back(generateRandomBlob<32>());
  //index.chainWork = generateRandomBytesVector(uint256::size());
  return index;
}

template <>
inline StoredBlockIndex<AltBlock> getRandomIndex() {
  StoredBlockIndex<AltBlock> index;
  index.height = rand();

  AltBlock block;
  block.hash = generateRandomBytesVector(12);
  block.previousBlock = generateRandomBytesVector(12);
  block.timestamp = rand();
  block.height = rand();

  index.header = std::make_shared<AltBlock>(block);
  index.addon._vtbids.push_back(generateRandomBlob<32>());
  index.addon._vtbids.push_back(generateRandomBlob<32>());
  index.addon._atvids.push_back(generateRandomBlob<32>());
  index.addon._atvids.push_back(generateRandomBlob<32>());
  index.addon._vbkblockids.push_back(
      generateRandomBlob<VbkBlock::short_hash_t::size()>());
  index.addon._vbkblockids.push_back(
      generateRandomBlob<VbkBlock::short_hash_t::size()>());
  index.status = rand() & 0x0f;

  return index;
}

struct AltChainParamsTest : public AltChainParams {
  AltBlock getBootstrapBlock() const noexcept override {
    AltBlock genesisBlock;
    genesisBlock.hash = std::vector<uint8_t>(10, 1);
    genesisBlock.previousBlock = std::vector<uint8_t>(10, 2);
    genesisBlock.height = 0;
    genesisBlock.timestamp = 0;
    return genesisBlock;
  }

  int64_t getIdentifier() const noexcept override { return 0x7ec7; }

  std::vector<uint8_t> getHash(
      const std::vector<uint8_t>& bytes) const noexcept override {
    return AssertDeserializeFromRaw<AltBlock>(bytes, *this).getHash();
  }

  bool checkBlockHeader(const std::vector<uint8_t>&,
                        const std::vector<uint8_t>&,
                        ValidationState&) const noexcept override {
    return true;
  }
};

}  // namespace altintegration

#endif
