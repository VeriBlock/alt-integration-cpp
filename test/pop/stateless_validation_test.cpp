// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>

#include <veriblock/pop/blockchain/alt_chain_params.hpp>
#include <veriblock/pop/blockchain/btc_chain_params.hpp>
#include <veriblock/pop/blockchain/vbk_chain_params.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/literals.hpp>
#include <veriblock/pop/pop_stateless_validator.hpp>
#include <veriblock/pop/stateless_validation.hpp>

#include "util/pop_test_fixture.hpp"
#include "util/test_utils.hpp"

using namespace altintegration;

static const ATV validATV = AssertDeserializeFromHex<ATV>(
    "0000000101dcbb01011667b55493722b4807bea7bb8ed2835d990885f3fe51c30203e80001"
    "0701ba0100016a20636ff0ad45e996d94acb675205c98ad797ce634f76091ecac408cd7da6"
    "3e68092032c4ff054ad6f9e99460348e083a6237d2cd448bb8a8a6cf54bbe903b99e70eb00"
    "00000500000005fd94ec804d0b703debf5411271360fee6996e42b8c8ebd10a2e2550a1888"
    "7735013e000000050c0101010101010101010101010c010101010101010101010101000000"
    "0000000000000000000000000000000000000000000000000000000000010a010203040506"
    "0708090a46304402207ff4cb421f3c5a17340606dff87ecdeb2cadb4d19258b70769c3feca"
    "5156af5702207386ff8653d4b4e1a764f8b419ce9d8416b316f91fb54ef10f5d36682d6536"
    "c5583056301006072a8648ce3d020106052b8104000a034200042fca63a20cb5208c2a55ff"
    "5099ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703d37cbd02ecd9c97a"
    "806110fa05d9014a102a0513dd354ec50400000000040000000020f16bf1d4cb1d617730bb"
    "afbeeaec5716fb78e5d040e4b3338083e97556e4ce23040000000041000000430002e4edf0"
    "9186bdf8f39469a1d402508210a3c4e2cf37a767f180c4c118958bf16bf1d4cb1d617730bb"
    "afbeeaec57165f8c848b010100000000000000");

static const VTB validVTB = AssertDeserializeFromHex<VTB>(
    "00000001020dbfbb02011667b55493722b4807bea7bb8ed2835d990885f3fe51c341000000"
    "0500026edd7a83dfbd4b56cad65ed577a1d330ec8c16fd6900000000000000000070e93ea1"
    "41e1fc673e017e97eadc6b965c9b949401010000000000000001510000000500026edd7a83"
    "dfbd4b56cad65ed577a1d330ec8c16fd6900000000000000000070e93ea141e1fc673e017e"
    "97eadc6b965c9b94940101000000000000008b334de3af7d2ba37c2efc1334e5b4fe011304"
    "0000000004000000000400000004000000205001000000eb5a665201d3e458eee75b74493f"
    "a91a7557a7f4375bd349c0ecb59691762a50842b620dbba278308a18de01ce6ae1fff526b6"
    "512a4f0c662ee22b9cb35b085e94949b5cffff7f202f0000000128500100000006226e4611"
    "1a0b59caaf126043eb5bbf28c34f3a5e332a1fc7b2b73cf188910ff57d9bba19226fbb5c15"
    "474850e3cd333baf101bf2acd4aadc099e112464703b94949b5cffff7f2002000000500100"
    "0000dca5bf5226bcc95c07d027441cb1e7d79796d1e4ab5e721a02b4b519df4cb33e056ccb"
    "fa774e5fbc343d4d79e0383298e84ecab5131ba4f829f8237f1c1ae10b94949b5cffff7f20"
    "000000005001000000bfaeff443812368985c2d1100d4eee8f8af09f0146ceefc5ec8bc94d"
    "9328332da94e82fd4915324f9ee4f0e2afb39436d35c043b73b570bea2551aaa2b2186ac94"
    "949b5cffff7f2000000000500100000006b1cfe9bac72c2d4159d3bf19c8210667c1d9540c"
    "3e9ee70dcedecae2762f24cdaf33c406e542099b2dd74c8ef35bac44a8180ed5bc0a9a4854"
    "298ab2d4700894949b5cffff7f200000000050010000006674f70816611fcb69e476fdc7ff"
    "7b670f885c352ac0e3a51579db8d383b8357c386ba20b90becf40407b2fb6c852320f630c4"
    "4aec1c32de4776d4ad2d7f84a394949b5cffff7f2003000000500100000007526cf73af2f9"
    "5cc4d359af0614a9dc4ddba2bd446d72849d14cfd4289fe82dfa7844bcb4be019405f6a810"
    "f2a05ef734d9d314e3a35099b73367d9ecf1053e94949b5cffff7f20050000005001000000"
    "fd40204242ab400ca343d2c0ac3da57bee07942179d32dda7b83ab5619bbef56e5e9bc9bee"
    "445732909931fb943a538448b325517e4c7e6a69da1ad023e6694994949b5cffff7f200700"
    "000050010000006c3357fc0ec3e59e07d1a3ad2c87fde9da78deecceef0d3a84103cdda226"
    "69638a5a70657587a8d998648266d4e835d854fa85d0b2d2aa60535ee2e9f508cf2594949b"
    "5cffff7f200b0000005001000000242b6cdb9eaf3378a83a361b23913f896286d6fd36c098"
    "fa05d352813bdc4d2cbb7cc07cf1660c8cf185e417ec809586acad51d359574e89a57cde44"
    "2980623f94949b5cffff7f200000000050010000005db28060551d0d7d279f98cca330119d"
    "2e5044da3ddd12896edcba1e0545fd154fef8d2d34116db81f07ac931627afff3a2f69b483"
    "bc62af630b61da66e4fc2294949b5cffff7f200d0000005001000000036b48e472f22d90eb"
    "1cb000a5f2daa5f7af8ed8b500b64cce741b4110394d031ff4c196a5d109779df80ae5d802"
    "3945ec1296edd8678324e4c7c13563b6d48994949b5cffff7f2013000000500100000013ad"
    "f99b975b3752f1b531ac9074e2cd0393fdedf0a07da0302970a17cd2db5e5e80c3c961cf08"
    "cb2a37c2b299caa8b4f2a67aacba6816cd90ae496cca82aa9594949b5cffff7f2015000000"
    "50010000007ad1645cd67ea0df1b0f67d80cf07dc761f0aa3b5e3962c4d6016f031381407d"
    "ff01c691a345cdd9e4fed118d49a5622008b794c99d3d2ed196ad74cda28036e94949b5cff"
    "ff7f20000000005001000000144078a90307ca1295b43c458be7520a4457d5fb39ca2035dc"
    "b01fc383551d193ace4b71cf49ab3ea66571cd8172fc699e62137c61870215ee2f430115d9"
    "2ac994949b5cffff7f20160000005001000000260576e4e6c25191ae1e72efcc12713f8874"
    "fd7d9361640153a28b10c26cf4613ca45bc76a8c7cf9bd324e7e8ce90dbf679bc3fefd61eb"
    "81ff647eea764fa77594949b5cffff7f2019000000500100000044ff5df22d0a816e855371"
    "6898cb9498affe5db0990338bceedb1acc1b55d966be0da7bcd103610a98d48e9f175b5099"
    "cf678b1000f04d01f3ec1672edb2f40294949b5cffff7f2000000000500100000040883ba4"
    "4e2f2c71eaaf636897045038e34247b0c057baebb8c7e86fe14e2e4f0cde97701f8ac963b9"
    "c602af2d2e218e16c53e7df6d6f2e5d60198d50eabbf9b94949b5cffff7f201b0000005001"
    "0000006a2d3f29d2e403b0d740a8e611f2afb61720d9f075cbb5530baf40303b77e833c2dd"
    "336cd155d561354b98d292d2cfe3a4a1c2168bfdd80d07011b212b1a895694949b5cffff7f"
    "201c00000050010000006dd3c8fa32c410953fae16c9f55e94cd1d813eef15b7f916070ff3"
    "8b30f2cd2b686afbaca81e793b4c2466ebee1a531c88814da4ddabe2c720e5ef13deedf7bc"
    "94949b5cffff7f201d0000005001000000763ec685ccab0db5480684e9f5bfe4acbb9655ab"
    "b30d26f4eff7dd5c51b2c33704db0811739e16c7f6f84ebaab2a54c03b3a6c1fb2ebd20d0e"
    "262b47064658f394949b5cffff7f201e0000005001000000a0253929a0b24d16ac1bc14ccd"
    "75d29f74e6f992685786eae8dcd60401aa7c7b539cebf2980fea869c4c70d55678879bcd5d"
    "470d220da0035bb5314da70a226094949b5cffff7f200000000050010000008006b4ca221c"
    "703f712ad12e0896d9390e7f08c884acbb1101973b0c3992ec345b76ce4e233263319a5447"
    "13b7fba3e2a52a5a095dfdc250dbb5b0d85afaa70d94949b5cffff7f200000000050010000"
    "00713c1d2f1f128cf6a25108e85ef264b0c7065599fa538c46834b5c9b6c370749c07be94a"
    "1f737cd14f4a6d1eb019d79c62dcf98037cf252e71286b214db6717594949b5cffff7f201f"
    "0000005001000000cb7038a95ea451e710076eb1df06a19514b555147898c8ca98b68cb7ff"
    "d92308dbd2308012b597c895244cc4d40257b6523ddeb5db02bc5bcaed369c7b0d315a9494"
    "9b5cffff7f2024000000500100000027d9eaf65e4be6a510ee79ae9cb674a01a57ee8e71cb"
    "260ec708e5721cc48a2b814902206e2ff0ee1d3b5755a63309e15e078aa8b54dcaff71c843"
    "16fd56e06194949b5cffff7f200000000050010000003bcb5dfe5825df24f4604add988eb4"
    "30a76c5251d1d51e21ae572142b2fef2592d96484b15fe49f4901a04a2fdc84ba822189f41"
    "b998b611e36947e3f87fa0e294949b5cffff7f2000000000500100000087dae66372419284"
    "26e5d6aa7acc41cf80cbc023c2d8ee5acbb682e36f6b584fea671502397fbaee6abca4752c"
    "8a718a8da93fe5862746456dae8f5ccb2515e894949b5cffff7f20000000005001000000b3"
    "90bbfbab98291a12947b7acb4e1cde5034976d64a33f1ee42c17afb458c96ca174db768a74"
    "60873be1cd4d7711a802e51e905b90e71cab61f5641b87b68c2b94949b5cffff7f20290000"
    "00500100000024afddffef180ef6fa667a98f9b707e35975882d088861631de4b61feecc80"
    "20d12fdd698e698d18df19b757ded68a915e79e85c94cacc393ae41d8f83282a6894949b5c"
    "ffff7f20000000005001000000921c6bcfd3a4aaa566b6aa8e7ae935ed091ed5d93789454e"
    "9d140b8d9ca3a140ccd322c402f3e699657d0c4d9ef3964715c0bc8461439b05aed1397597"
    "54570894949b5cffff7f20000000005001000000c154860be4151e92d3d7ca13e727db7f7b"
    "e62e1f56be34e260e6311d4640c72a6076d7e6aa03c4e601d000676b825b1de468879d53c7"
    "e0cf669d3461ef63287a94949b5cffff7f20000000005001000000630f5cb6dd49da9abee9"
    "6c3438bcc1546bedd0ffbcda626e21366882c546ad0b3679d83718626032b75d6ed15c9dd1"
    "f4324e99156a308dcdddc6ed0d604fd9ff94949b5cffff7f202a0000005001000000520a5a"
    "7190eb62b2fb393038b335202babf55cbe09cbef231f31f380180ff83c82e200d0ab87f774"
    "6f95133ddeb6cf0d5a313c66ffedcdea839c5cb5bf96c8b294949b5cffff7f200000000050"
    "01000000ccd2e9907adf9dcfc69f932ee11b0337750c64014e29e3da36ba75e376117e0ebb"
    "f537f27355f2a3a96aab35fb1622be386bef2b11beefabbf01ded57e4178c994949b5cffff"
    "7f2000000000500100000007c1358473941c714d172c9dd2ae9709535d362d659b44b00481"
    "ed3ae895964478b55ac9fa659207f23d154f93aba35eaf8d3cf155d0012abf433b14843a36"
    "af94949b5cffff7f20000000005001000000f4cee631258e04c5f5312785bd3eab5dfac2ac"
    "e997c25b0e42ec67ccac458c3f6c735adcfaa48231aa1d9fa32b628ddccfe2387133ab3542"
    "5564402ba5fdef9094949b5cffff7f20000000005001000000cd43104433d16acb475111c4"
    "ab812adb80b4d1057ed8603677ee5432ff8939362b9c8ce12f1987531ee3d1ed38b44d9589"
    "eb08ad1c2575abf2ca76b07411704994949b5cffff7f202c00000050010000006dd660d712"
    "3c108014e379a682f5c78b4b1f29b01d063c32e5f21e92e2aee975cf11bad3e61e4705ef2e"
    "7d9b109aca236215d5cc8ceac4dece9f6923ac9fadfc94949b5cffff7f202e000000500100"
    "0000b5f54cbefe1e7aa47e19db9b2d94ec630956a64cc3c31fe167d934d0030fe367306623"
    "c896110dc32bef7b26004da8643d0e9a1aac8544dff95a1b1a8b7cb16894949b5cffff7f20"
    "000000005001000000707ca69542d05336234b32301cf999270d67293a2ef021959bf77809"
    "e2f7a94a986039c430d3a1683c905a79656bfe3f6bb1960774886c5adce715fdeebbe2d494"
    "949b5cffff7f2000000000473045022100cadfa30349b4a3f76f55a8d6623d37c7c3d770f7"
    "4fd710a6f038af9091a5b6c1022048c9a2012815ef84a3e9fdb43246e26e34e6a8a36f26e7"
    "d1219500137da206c2583056301006072a8648ce3d020106052b8104000a034200042fca63"
    "a20cb5208c2a55ff5099ca1966b7f52e687600784d1de062c1dd9c8a5fe55b2ba5d906c703"
    "d37cbd02ecd9c97a806110fa05d9014a102a0513dd354ec504000000000400000000206bf3"
    "d41c8059c843d202fd75631d9ded1d9978dff61b9f21a8f6a3269867559604000000004100"
    "00000b0002f1606ebb9390033c3532bceb77a1d330ec8c16fd690000000000000000006bf3"
    "d41c8059c843d202fd75631d9ded5c9b9494010100000000000000");

struct StatelessValidationTest : public ::testing::Test, public PopTestFixture {
  BtcChainParamsRegTest btc;
  VbkChainParamsRegTest vbk;
  AltChainParamsRegTest alt;
  ValidationState state;
};

TEST_F(StatelessValidationTest, checkBtcBlock_when_valid_test) {
  ASSERT_TRUE(checkBlock(validVTB.transaction.blockOfProof, state, btc))
      << state.toString();
}

TEST_F(StatelessValidationTest,
       checkBtcBlock_when_checkProofOfWork_invalid_test) {
  BtcBlock block = validVTB.transaction.blockOfProof;
  block.setNonce(1337);
  ASSERT_FALSE(checkProofOfWork(block, btc));
}

TEST_F(StatelessValidationTest, check_valid_vbk_block) {
  ASSERT_TRUE(checkBlock(validVTB.containingBlock, state, vbk))
      << state.toString();
}

TEST_F(StatelessValidationTest,
       checkVbkBlock_when_checkProofOfWork_invalid_test) {
  VbkBlock block = validVTB.containingBlock;
  block.setDifficulty(999999);
  ASSERT_FALSE(checkProofOfWork(block, vbk));
}

TEST_F(StatelessValidationTest, ATV_valid) {
  AltChainParamsRegTest altp;
  ASSERT_TRUE(checkATV(validATV, state, altp, vbk)) << state.toString();
}

TEST_F(StatelessValidationTest,
       ATV_checkMerklePath_different_transaction_invalid) {
  ATV atv = validATV;
  atv.merklePath.subject = uint256(
      "5B977EA09A554AD56957F662284044E7D37450DDADF7DB3647712F5969399787"_unhex);

  ASSERT_FALSE(checkMerklePath(atv.merklePath,
                               atv.transaction.getHash(),
                               atv.blockOfProof.getMerkleRoot(),
                               state));
}

TEST_F(StatelessValidationTest,
       ATV_checkMerklePath_merkleRoot_dont_match_ivalid) {
  ATV atv = validATV;
  atv.blockOfProof.setMerkleRoot(
      uint128("0356EB39B851682679F9A0131A4E4A5F"_unhex));

  ASSERT_FALSE(checkMerklePath(atv.merklePath,
                               atv.transaction.getHash(),
                               atv.blockOfProof.getMerkleRoot(),
                               state))
      << state.toString();
}

TEST_F(StatelessValidationTest, VTB_valid) {
  ASSERT_TRUE(checkVTB(validVTB, state, btc, vbk)) << state.toString();
}

TEST_F(StatelessValidationTest,
       VTB_checkMerklePath_different_transaction_invalid) {
  VTB vtb = validVTB;
  vtb.merklePath.subject = uint256(
      "3A014E88ED7AB65CDFAA85DAEAB07EEA6CBA5E147F736EDD8D02C2F9DDF0DEC6"_unhex);

  ASSERT_FALSE(checkMerklePath(vtb.merklePath,
                               vtb.transaction.getHash(),
                               vtb.containingBlock.getMerkleRoot(),
                               state));
}

TEST_F(StatelessValidationTest,
       VTB_checkMerklePath_merkleRoot_dont_match_ivalid) {
  VTB vtb = validVTB;
  vtb.containingBlock.setMerkleRoot(
      uint128("0356EB39B851682679F9A0131A4E4A5F"_unhex));
  ASSERT_FALSE(checkMerklePath(vtb.merklePath,
                               vtb.transaction.getHash(),
                               vtb.containingBlock.getMerkleRoot(),
                               state));
}

TEST_F(StatelessValidationTest, checkVbkPopTx_valid) {
  ASSERT_TRUE(checkVbkPopTx(validVTB.transaction, state, btc, vbk))
      << state.toString();
}

TEST_F(StatelessValidationTest, VbkPopTx_checkSignature_signature_invalid) {
  VbkPopTx tx = validVTB.transaction;
  tx.signature =
      "30450220034DC73796E9870E6679F47E48F3AC794327FD19D9023C228CD134D8ED87B796"
      "022100AD0CE8A520AAE704447920CA365D57A881A82A7455293A9C10E622E0BDD732AF"_unhex;
  ASSERT_FALSE(checkSignature(tx, state));
}

TEST_F(StatelessValidationTest, VbkPopTx_different_address_invalid) {
  VbkPopTx tx = validVTB.transaction;
  tx.publicKey =
      "3056301006072A8648CE3D020106052B8104000A03420004DE4EE8300C3CD99E913536CF53C4ADD179F048F8FE90E5ADF3ED19668DD1DBF6C2D8E692B1D36EAC7187950620A28838DA60A8C9DD60190C14C59B82CB90319E"_unhex;
  ASSERT_FALSE(checkSignature(tx, state));
}

TEST_F(StatelessValidationTest, checkBitcoinTransactionForPoPData_invalid) {
  VbkPopTx tx = validVTB.transaction;
  tx.publishedBlock = AssertDeserializeFromRaw<VbkBlock>(
      "00001388000294E7DC3E3BE21A96ECCF0FBDF5F62A3331DC995C36B0935637860679DDD5DB0F135312B2C27867C9A83EF1B99B985C9B949307023AD672BAFD7700"_unhex);
  ASSERT_FALSE(checkBitcoinTransactionForPoPData(tx, state));
}

TEST_F(StatelessValidationTest,
       checkBitcoinMerklePath_different_transaction_invalid) {
  VbkPopTx tx = validVTB.transaction;
  tx.merklePath.subject = uint256(
      "012A4E88ED7AB65CDFAA85DAEAB07EEA6CBA5E147F736EDD8D02C2F9DDF0DEC6"_unhex);
  ASSERT_FALSE(checkMerklePath(tx.merklePath,
                               tx.bitcoinTransaction.getHash(),
                               tx.blockOfProof.getMerkleRoot(),
                               state));
}

TEST_F(StatelessValidationTest,
       checkBitcoinMerklePath_merkle_root_does_not_match_invalid) {
  VbkPopTx tx = validVTB.transaction;
  tx.blockOfProof = AssertDeserializeFromRaw<BtcBlock>(
      "00000020BAA42E40345A7F826A31D37DB1A5D64B67B72732477422000000000000000000"
      "A33AD6BE0634647B26633AB85FA8DE258480BBB25E59C68E48BB0B608B12362B10919B5C"_unhex
      "6C1F2C1749C4D1F0");
  ASSERT_FALSE(checkMerklePath(tx.merklePath,
                               tx.bitcoinTransaction.getHash(),
                               tx.blockOfProof.getMerkleRoot(),
                               state));
}

TEST_F(StatelessValidationTest, checkBitcoinBlocks_when_not_contiguous) {
  VbkPopTx tx = validVTB.transaction;
  tx.blockOfProofContext.erase(tx.blockOfProofContext.begin() + 1);
  ASSERT_FALSE(checkBtcBlocks(tx.blockOfProofContext, state, btc));
}

TEST_F(StatelessValidationTest, checkVbkTx_valid) {
  ASSERT_TRUE(checkVbkTx(validATV.transaction, alt, vbk, state))
      << state.toString();
}

TEST_F(StatelessValidationTest, VbkTx_checkSignature_signature_invalid) {
  VbkTx tx = validATV.transaction;
  tx.signature =
      "30440220398B74708DC8F8AEE68FCE0C47B8959E6FCE6354665DA3ED87583F708E62AA6B02202E6C00C00487763C55E92C7B8E1DD538B7375D8DF2B2117E75ACBB9DB7DEB3C7"_unhex;
  ASSERT_FALSE(checkVbkTx(tx, alt, vbk, state));
}

TEST_F(StatelessValidationTest, VbkTx_different_address_invalid) {
  VbkTx tx = validATV.transaction;
  tx.publicKey =
      "3056301006072A8648CE3D020106552B8104000A03420004DE4EE8300C3CD99E913536CF53C4ADD179F048F8FE90E5ADF3ED19668DD1DBF6C2D8E692B1D36EAC7187950620A28838DA60A8C9DD60190C14C59B82CB90319E"_unhex;
  ASSERT_FALSE(checkSignature(tx, state));
}

TEST_F(StatelessValidationTest, containsSplit_when_descriptor_before_chunks) {
  srand(0);

  WriteStream buffer;
  buffer.write(generateRandomBytesVector(15));

  // Descriptor bytes (3 MAGIC, 1 SIZE, 7 SECTIONALS)
  buffer.write("927A594624509D41F548C0"_unhex);

  buffer.write(generateRandomBytesVector(10));

  // First chunk of 20 bytes
  buffer.write("00000767000193093228BD2B4906F6B84BE5E618"_unhex);

  buffer.write(generateRandomBytesVector(39));

  // Second chunk of 20 bytes
  buffer.write("09C0522626145DDFB988022A0684E2110D384FE2"_unhex);

  buffer.write(generateRandomBytesVector(31));

  // Third chunk of 21 bytes
  buffer.write("BFD38549CB19C41893C258BA5B9CAB24060BA2D410"_unhex);

  buffer.write(generateRandomBytesVector(35));

  // Fourth chunk of unstated 19 bytes
  buffer.write("39DFC857801424B0F5DE63992A016F5F38FEB4"_unhex);

  buffer.write(generateRandomBytesVector(22));

  ASSERT_TRUE(containsSplit(
      "00000767000193093228BD2B4906F6B84BE5E61809C0522626145DDFB988022A0684E2110D384FE2BFD38549CB19C41893C258BA5B9CAB24060BA2D41039DFC857801424B0F5DE63992A016F5F38FEB4"_unhex,
      buffer.data(),
      state))
      << state.toString();
}

TEST_F(StatelessValidationTest, containsSplit_when_chunked) {
  srand(0);

  WriteStream buffer;
  buffer.write(generateRandomBytesVector(15));

  // Descriptor bytes (3 MAGIC, 1 SIZE, 7 SECTIONALS)
  buffer.write("927A594624509D41F548C0"_unhex);

  buffer.write(generateRandomBytesVector(10));

  buffer.write("00000767000193093228BD2B4906F6B84BE5E618"_unhex);

  buffer.write(generateRandomBytesVector(39));

  // Second chunk of 20 bytes
  buffer.write("09C0522626145DDFB988022A0684E2110D384FE2"_unhex);

  buffer.write(generateRandomBytesVector(31));

  // Third chunk of 21 bytes
  buffer.write("BFD38549CB19C41893C258BA5B9CAB24060BA2D410"_unhex);

  buffer.write(generateRandomBytesVector(35));

  // Fourth chunk of unstated 19 bytes
  buffer.write("39DFC857801424B0F5DE63992A016F5F38FEB4"_unhex);

  buffer.write(generateRandomBytesVector(22));

  ASSERT_TRUE(containsSplit(
      "00000767000193093228BD2B4906F6B84BE5E61809C0522626145DDFB988022A0684E2110D384FE2BFD38549CB19C41893C258BA5B9CAB24060BA2D41039DFC857801424B0F5DE63992A016F5F38FEB4"_unhex,
      buffer.data(),
      state))
      << state.toString();
}

TEST_F(StatelessValidationTest, parallel_check_valid_vbk_block) {
  PopValidator validator(vbk, btc, alt);
  auto result = validator.addCheck(validVTB.containingBlock);
  ASSERT_TRUE(result.get().IsValid()) << result.get().toString();
}

TEST_F(StatelessValidationTest, parallel_check_invalid_vbk_block) {
  PopValidator validator(vbk, btc, alt);
  VbkBlock block = validVTB.containingBlock;
  block.setDifficulty(999999);
  auto result = validator.addCheck(block);
  ASSERT_FALSE(result.get().IsValid()) << result.get().toString();
}

TEST_F(StatelessValidationTest, parallel_check_mixed_vbk_block) {
  PopValidator validator(vbk, btc, alt);
  std::vector<std::future<ValidationState>> results;
  results.push_back(validator.addCheck(validVTB.containingBlock));
  VbkBlock block = validVTB.containingBlock;
  block.setDifficulty(999999);
  results.push_back(validator.addCheck(block));
  ASSERT_FALSE(results.back().get().IsValid())
      << results.back().get().toString();
  results.pop_back();
  ASSERT_TRUE(results.back().get().IsValid())
      << results.back().get().toString();
}

TEST_F(StatelessValidationTest, parallel_check_valid_vtb) {
  PopValidator validator(vbk, btc, alt);
  auto result = validator.addCheck(validVTB);
  ASSERT_TRUE(result.get().IsValid()) << result.get().toString();
}

TEST_F(StatelessValidationTest, parallel_check_valid_atv) {
  PopValidator validator(vbk, btc, alt);
  auto result = validator.addCheck(validATV);
  ASSERT_TRUE(result.get().IsValid()) << result.get().toString();
}

TEST_F(StatelessValidationTest, parallel_check_valid_pop) {
  PopValidator validator(vbk, btc, alt);
  PopData pop{};
  pop.context.push_back(validVTB.containingBlock);
  pop.vtbs.push_back(validVTB);
  pop.atvs.push_back(validATV);
  bool result = checkPopData(validator, pop, state);
  ASSERT_TRUE(result) << state.toString();
  ASSERT_TRUE(state.IsValid()) << state.toString();
}

TEST_F(StatelessValidationTest, parallel_check_invalid_pop) {
  PopValidator validator(vbk, btc, alt);
  PopData pop{};
  VbkBlock block = validVTB.containingBlock;
  block.setDifficulty(999999);
  pop.context.push_back(block);
  pop.vtbs.push_back(validVTB);
  pop.atvs.push_back(validATV);
  bool result = checkPopData(validator, pop, state);
  ASSERT_FALSE(result) << state.toString();
  ASSERT_FALSE(state.IsValid()) << state.toString();
  ASSERT_EQ(state.GetPathParts().front(), "pop-sl-invalid");

  // clear state and try validating again to make sure validator's state does
  // not matter
  PopData pop2{};
  state = ValidationState();
  pop2.context.push_back(validVTB.containingBlock);
  pop2.vtbs.push_back(validVTB);
  pop2.atvs.push_back(validATV);
  result = checkPopData(validator, pop2, state);
  ASSERT_TRUE(result) << state.toString();
  ASSERT_TRUE(state.IsValid()) << state.toString();
}

TEST_F(StatelessValidationTest, parallel_check_invalid_pop_size) {
  PopValidator validator(vbk, btc, alt);
  PopData pop{};
  VbkBlock block = validVTB.containingBlock;
  pop.context = std::vector<VbkBlock>(150000, block);
  bool result = checkPopData(validator, pop, state);
  ASSERT_FALSE(result) << state.toString();
  ASSERT_FALSE(state.IsValid()) << state.toString();
  ASSERT_EQ(state.GetPathParts().front(), "pop-sl-oversize");
}

TEST(VbkBlockPlausibility, Height) {
  const VbkChainParamsMain param;
  const auto forkHeight = param.getProgPowForkHeight();
  const auto forkTimestamp = param.getProgPowStartTimeEpoch();
  ValidationState state;
  VbkBlock block;
  // set valid timestamp
  block.setTimestamp(forkTimestamp);

  // too low
  block.setHeight(forkHeight - 1);
  EXPECT_FALSE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "height-too-low");
  state.reset();

  // ok
  block.setHeight(forkHeight);
  EXPECT_TRUE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  state.reset();

  // too high
  const auto firstInvalidHeight =
      VBK_ETHASH_EPOCH_LENGTH * (VBK_MAX_CALCULATED_EPOCHS_SIZE + 1);
  block.setHeight(firstInvalidHeight);
  // set valid timestamp
  block.setTimestamp(forkTimestamp * 3 / 2);
  EXPECT_FALSE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "height-too-high");
  state.reset();
}

TEST(VbkBlockPlausibility, Timestamp) {
  const VbkChainParamsMain param;
  const auto forkHeight = param.getProgPowForkHeight();
  const auto forkTimestamp = param.getProgPowStartTimeEpoch();
  ValidationState state;
  VbkBlock block;
  // set valid height
  block.setHeight(forkHeight);

  // lower than progpow start time epoch
  block.setTimestamp(forkTimestamp - 1);
  EXPECT_FALSE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "timestamp-too-low");
  state.reset();

  // too low
  block.setHeight(forkHeight * 2);
  block.setTimestamp(forkTimestamp);  // valid but under expected timestamp
  EXPECT_FALSE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "timestamp-lower-bound");
  state.reset();

  // ok, right at upper bound for height=(forkHeight * 2)
  const auto upperBound = 1655580052;
  block.setTimestamp(upperBound);
  EXPECT_TRUE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  state.reset();

  // too high
  block.setTimestamp(upperBound + 1);
  EXPECT_FALSE(checkVbkBlockPlausibility(block, state, param))
      << state.toString();
  EXPECT_EQ(state.GetPath(), "timestamp-upper-bound");
  state.reset();
}