#include <gtest/gtest.h>

#include <veriblock/pop/storage/endorsement_repository_inmem.hpp>
#include <veriblock/pop/storage/endorsement_repository_rocks.hpp>
#include <veriblock/pop/storage/repository_rocks_manager.hpp>

using namespace altintegration;

// DB name
static const std::string dbName = "db-endorsement_storage_test";

static const std::vector<uint8_t> defaultVtbEncoded = ParseHex(
    "02046002011667ff0a897e5d512a0b6da2f41c479867fe6b3a4cae2640000013350002a793"
    "c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95f0a15628b06ba3"
    "b44c0190b5c0495c9b8acd0701c5235ebbbe9c02011b01000000010ce74f1fb694a001eebb"
    "1d7d08ce6208033f5bf7263ebad2de07bbf518672732000000006a47304402200cf4998aba"
    "1682abeb777e762807a9dd2635a0b77773f66491b83ee3c87099ba022033b7ca24dc520915"
    "b8b0200cbdcf95ba6ae866354585af9c53ee86f27362ebec012103e5baf0709c395a82ef0b"
    "d63bc8847564ac201d69a8e6bf448d87aa53a1c431aaffffffff02b7270d00000000001976"
    "a9148b9ea8545059f3a922457afd14ddf3855d8b109988ac0000000000000000536a4c5000"
    "0013350002a793c872d6f6460e90bed62342bb968195f8c515d3eed7277a09efac4be99f95"
    "f0a15628b06ba3b44c0190b5c0495c9b8acd0701c5235ebbbe9cd4e943efe1864df0421661"
    "5cf92083f40000000002019f040000067b040000000c040000000400000020204d66077fdf"
    "24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917"
    "ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307"
    "c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2ab"
    "da520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e43"
    "6f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37"
    "a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9"
    "a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb16220"
    "1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e9"
    "2ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f0"
    "28ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574b"
    "ddbf2c5db9174af5ad0d278fe92e06e4ac349a42500000c02000000000000000000014297d"
    "038cb54bfa964b44fb9a2d9853eb5936d4094f13a5e4a299b6c0cbdac21e997d74a999c26a"
    "cd68c34bdfb527b10ddd779a1a0bceb3919b5c6c1f2c1773703bc001035000008020000000"
    "0000000000000d5efbd7dc73f09e8aaf064d1a76142d4bac4e9dcc61fc255eefbc6d8670ee"
    "98c583aeed677f27fc239c41f93ee411add001b1d40815a3268b9b5c6c1f2c17e11874af50"
    "0000402000000000000000000013535112250e115e2896e4f602c353d839443080398e3f1d"
    "fb5f1e89109ef8508bde5404cf244a6372f402e0cf9d8dbd818326222ca739e08d9b5c6c1f"
    "2c1744290a9250000000200000000000000000002274473227b7674bd6a5b17dd3316a827f"
    "5a34402ea4ba2b36128b600bbb488ec6595eb2bb808425dea85fb83a63267b643406bed63a"
    "a310919b5c6c1f2c1749c4d1f0473045022100f4dce45edcc6bfc4a1f44ef04e47e90a348e"
    "fd471f742f18b882ac77a8d0e89e0220617cf7c4a22211991687b17126c1bb007a3b2a25c5"
    "50f75d66b857a8fd9d75e7583056301006072a8648ce3d020106052b8104000a03420004b3"
    "c10470c8e8e426f1937758d9fb5e97a1891176cb37d4c12d4af4107b1aa3e8a8a754c06a22"
    "760e44c60642fba883967c19740d5231336326f7962750c8df990400000000040000000d20"
    "2a014e88ed7ab65cdfaa85daeab07eea6cba5e147f736edd8d02c2f9ddf0dec60400000006"
    "205b977ea09a554ad56957f662284044e7d37450ddadf7db3647712f59693997872020d0a3"
    "d873eeeee6a222a75316dce60b53ca43eaea09d27f0ece897303a53ae920c06fe913dca5dc"
    "2736563b80834d69e6dfdf1b1e92383ea62791e410421b6c1120049f68d350eeb8b3df630c"
    "8308b5c8c2ba4cd6210868395b084af84d19ff0e9020000000000000000000000000000000"
    "00000000000000000000000000000000002036252dfc621de420fb083ad9d8767cba627edd"
    "eec64e421e9576cee21297dd0a40000013700002449c60619294546ad825af03b093563786"
    "0679ddd55ee4fd21082e18686eb53c1f4e259e6a0df23721a0b3b4b7ab5c9b9211070211ca"
    "f01c3f010100");

static const std::vector<uint8_t> defaultAtvEncoded = ParseHex(
    "01580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100"
    "010c6865616465722062797465730112636f6e7465787420696e666f206279746573011170"
    "61796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e"
    "6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d"
    "8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a034200"
    "04de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692"
    "b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e040000000104000000"
    "00201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000"
    "00022000000000000000000000000000000000000000000000000000000000000000002000"
    "00000000000000000000000000000000000000000000000000000000000000400000138800"
    "02449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4"
    "462ef24ae02d67e47d785c9b90f301010000000000010100");

template <typename EndorsementRepository_t>
std::shared_ptr<EndorsementRepository_t> getRepo();

template <>
std::shared_ptr<EndorsementRepositoryRocks<BtcEndorsement>> getRepo() {
  RepositoryRocksManager rocksManager(dbName);
  rocksManager.open();
  rocksManager.clear();
  return rocksManager.getBtcEndorsementRepo();
}

template <>
std::shared_ptr<EndorsementRepositoryRocks<VbkEndorsement>> getRepo() {
  RepositoryRocksManager rocksManager(dbName);
  rocksManager.open();
  rocksManager.clear();
  return rocksManager.getVbkEndorsementRepo();
}

template <>
std::shared_ptr<EndorsementRepositoryInmem<BtcEndorsement>> getRepo() {
  return std::make_shared<EndorsementRepositoryInmem<BtcEndorsement>>();
}

template <>
std::shared_ptr<EndorsementRepositoryInmem<VbkEndorsement>> getRepo() {
  return std::make_shared<EndorsementRepositoryInmem<VbkEndorsement>>();
}

template <typename Container_t>
Container_t getDefaultContainer();

template <typename Container_t>
Container_t getModifiedContainer();

template <>
VTB getDefaultContainer() {
  return VTB::fromVbkEncoding(defaultVtbEncoded);
}

template <>
AltPayloads getDefaultContainer() {
  AltPayloads payloads;

  AltProof container;
  container.endorsed = {{1, 2, 3}, {}, 123, 4125};
  container.containing = {{
                              11,
                              12,
                              24,
                          },
                          {},
                          1247,
                          1425};
  container.atv = ATV::fromVbkEncoding(defaultAtvEncoded);

  payloads.alt = container;

  return payloads;
}

template <>
VTB getModifiedContainer() {
  VTB vtb = VTB::fromVbkEncoding(defaultVtbEncoded);
  vtb.transaction.bitcoinTransaction = BtcTx({1, 2, 3});
  return vtb;
}

template <>
AltPayloads getModifiedContainer() {
  AltPayloads payloads;
  AltProof container;
  container.endorsed = {{1, 2, 3}, {}, 123, 4125};
  container.containing = {{
                              11,
                              12,
                              24,
                          },
                          {},
                          1247,
                          1425};
  container.atv = ATV::fromVbkEncoding(defaultAtvEncoded);

  container.atv.transaction.sourceAmount = Coin(213);
  payloads.alt = container;
  return payloads;
}

template <typename EndorsementRepository_t>
struct EndorsementRepositoryTest : public ::testing::Test {
  using endorsement_t = typename EndorsementRepository_t::endorsement_t;
  using eid_t = typename endorsement_t::id_t;
  using endorsed_hash_t = typename endorsement_t::endorsed_hash_t;
  using containing_hash_t = typename endorsement_t::containing_hash_t;
  using container_t = typename endorsement_t::container_t;
  using cursor_t = typename EndorsementRepository_t::cursor_t;

  std::shared_ptr<EndorsementRepository<endorsement_t>> endorsementRepo;

  EndorsementRepositoryTest() {
    endorsementRepo = getRepo<EndorsementRepository_t>();
  }
};

TYPED_TEST_SUITE_P(EndorsementRepositoryTest);

TYPED_TEST_P(EndorsementRepositoryTest, Basic) {
  typename Basic::container_t defaultContainer =
      getDefaultContainer<typename Basic::container_t>();

  this->endorsementRepo->put(defaultContainer);
  typename Basic::endorsement_t expectedEndorsement1 =
      Basic::endorsement_t::fromContainer(defaultContainer);

  typename Basic::container_t modifiedContainer =
      getModifiedContainer<typename Basic::container_t>();
  this->endorsementRepo->put(modifiedContainer);
  typename Basic::endorsement_t expectedEndorsement2 =
      Basic::endorsement_t::fromContainer(modifiedContainer);

  std::vector<typename Basic::endorsement_t> endorsements =
      this->endorsementRepo->get(expectedEndorsement1.endorsedHash);

  EXPECT_EQ(endorsements.size(), 2);

  if (endorsements[0].id != expectedEndorsement1.id) {
    std::reverse(endorsements.begin(), endorsements.end());
  }

  EXPECT_EQ(expectedEndorsement1.blockOfProof, endorsements[0].blockOfProof);
  EXPECT_EQ(expectedEndorsement1.containingHash,
            endorsements[0].containingHash);
  EXPECT_EQ(expectedEndorsement1.endorsedHash, endorsements[0].endorsedHash);
  EXPECT_EQ(expectedEndorsement1.id, endorsements[0].id);

  EXPECT_EQ(expectedEndorsement2.blockOfProof, endorsements[1].blockOfProof);
  EXPECT_EQ(expectedEndorsement2.containingHash,
            endorsements[1].containingHash);
  EXPECT_EQ(expectedEndorsement2.endorsedHash, endorsements[1].endorsedHash);
  EXPECT_EQ(expectedEndorsement2.id, endorsements[1].id);

  this->endorsementRepo->remove(expectedEndorsement2.id);
  endorsements = this->endorsementRepo->get(expectedEndorsement1.endorsedHash);

  EXPECT_EQ(endorsements.size(), 1);

  EXPECT_EQ(expectedEndorsement1.blockOfProof, endorsements[0].blockOfProof);
  EXPECT_EQ(expectedEndorsement1.containingHash,
            endorsements[0].containingHash);
  EXPECT_EQ(expectedEndorsement1.endorsedHash, endorsements[0].endorsedHash);
  EXPECT_EQ(expectedEndorsement1.id, endorsements[0].id);

  this->endorsementRepo->remove(defaultContainer);
  endorsements = this->endorsementRepo->get(expectedEndorsement1.endorsedHash);

  EXPECT_EQ(endorsements.size(), 0);
}

TYPED_TEST_P(EndorsementRepositoryTest, Cursor) {
  typename Cursor::container_t defaultContainer =
      getDefaultContainer<typename Cursor::container_t>();

  this->endorsementRepo->put(defaultContainer);
  typename Cursor::endorsement_t expectedEndorsement1 =
      Cursor::endorsement_t::fromContainer(defaultContainer);

  typename Cursor::container_t modifiedContainer =
      getModifiedContainer<typename Cursor::container_t>();
  this->endorsementRepo->put(modifiedContainer);
  typename Cursor::endorsement_t expectedEndorsement2 =
      Cursor::endorsement_t::fromContainer(modifiedContainer);

  std::vector<typename Cursor::endorsement_t> expectedEndorsements = {
      expectedEndorsement1, expectedEndorsement2};

  std::shared_ptr<typename Cursor::cursor_t> cursor =
      this->endorsementRepo->newCursor();

  std::vector<typename Cursor::endorsement_t> values;
  for (cursor->seekToFirst(); cursor->isValid(); cursor->next()) {
    values.push_back(cursor->value());
  }

  EXPECT_EQ(values.size(), expectedEndorsements.size());
  for (const auto& el : values) {
    EXPECT_TRUE(std::find(expectedEndorsements.begin(),
                          expectedEndorsements.end(),
                          el) != expectedEndorsements.end());
  }

  cursor->seekToFirst();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_EQ(cursor->key(), values[0].id);
  EXPECT_EQ(cursor->value(), values[0]);

  cursor->seekToLast();
  EXPECT_TRUE(cursor->isValid());
  EXPECT_EQ(cursor->key(), values[values.size() - 1].id);
  EXPECT_EQ(cursor->value(), values[values.size() - 1]);

  // read in reversed order
  std::vector<typename Cursor::endorsement_t> reversedValues;
  for (cursor->seekToLast(); cursor->isValid(); cursor->prev()) {
    reversedValues.push_back(cursor->value());
  }

  // then reverse it
  std::reverse(reversedValues.begin(), reversedValues.end());
  EXPECT_EQ(reversedValues, values);

  // find some values
  cursor->seek(expectedEndorsement1.id);
  EXPECT_TRUE(cursor->isValid());  // key found
  EXPECT_EQ(cursor->value(), expectedEndorsement1);

  // iterate before first element
  cursor->seekToFirst();
  cursor->prev();
  EXPECT_FALSE(cursor->isValid());

  // iterate after last element
  cursor->seekToLast();
  cursor->next();
  EXPECT_FALSE(cursor->isValid());

  cursor->seek(expectedEndorsement2.id);
  EXPECT_TRUE(cursor->isValid());  // key found
  EXPECT_EQ(cursor->value(), expectedEndorsement2);
}

REGISTER_TYPED_TEST_SUITE_P(EndorsementRepositoryTest, Basic, Cursor);

typedef ::testing::Types<EndorsementRepositoryRocks<BtcEndorsement>,
                         EndorsementRepositoryRocks<VbkEndorsement>,
                         EndorsementRepositoryInmem<BtcEndorsement>,
                         EndorsementRepositoryInmem<VbkEndorsement>>
    TypesUnderTest;

INSTANTIATE_TYPED_TEST_SUITE_P(EndorsementStorageTestSuit,
                               EndorsementRepositoryTest,
                               TypesUnderTest);

TEST(EndorsementRepository, copy_constructor_test1) {
  std::shared_ptr<EndorsementRepositoryRocks<BtcEndorsement>> rocksRepo1 =
      getRepo<EndorsementRepositoryRocks<BtcEndorsement>>();
  std::shared_ptr<EndorsementRepositoryInmem<BtcEndorsement>> inmemRepo1 =
      getRepo<EndorsementRepositoryInmem<BtcEndorsement>>();

  typename BtcEndorsement::container_t defaultContainer =
      getDefaultContainer<typename BtcEndorsement::container_t>();
  BtcEndorsement expectedEndorsement1 =
      BtcEndorsement::fromContainer(defaultContainer);

  rocksRepo1->put(defaultContainer);

  typename BtcEndorsement::container_t modifiedContainer =
      getModifiedContainer<typename BtcEndorsement::container_t>();
  BtcEndorsement expectedEndorsement2 =
      BtcEndorsement::fromContainer(modifiedContainer);

  rocksRepo1->put(modifiedContainer);

  endorsementRepositoryCopy(*rocksRepo1, *inmemRepo1);

  std::vector<BtcEndorsement> endorsements =
      inmemRepo1->get(expectedEndorsement1.endorsedHash);

  EXPECT_EQ(endorsements.size(), 2);

  if (endorsements[0].id != expectedEndorsement1.id) {
    std::reverse(endorsements.begin(), endorsements.end());
  }

  EXPECT_EQ(expectedEndorsement1.blockOfProof, endorsements[0].blockOfProof);
  EXPECT_EQ(expectedEndorsement1.containingHash,
            endorsements[0].containingHash);
  EXPECT_EQ(expectedEndorsement1.endorsedHash, endorsements[0].endorsedHash);
  EXPECT_EQ(expectedEndorsement1.id, endorsements[0].id);

  EXPECT_EQ(expectedEndorsement2.blockOfProof, endorsements[1].blockOfProof);
  EXPECT_EQ(expectedEndorsement2.containingHash,
            endorsements[1].containingHash);
  EXPECT_EQ(expectedEndorsement2.endorsedHash, endorsements[1].endorsedHash);
  EXPECT_EQ(expectedEndorsement2.id, endorsements[1].id);
}

// vise versa
TEST(EndorsementRepository, copy_constructor_test2) {
  std::shared_ptr<EndorsementRepositoryRocks<BtcEndorsement>> rocksRepo1 =
      getRepo<EndorsementRepositoryRocks<BtcEndorsement>>();
  std::shared_ptr<EndorsementRepositoryInmem<BtcEndorsement>> inmemRepo1 =
      getRepo<EndorsementRepositoryInmem<BtcEndorsement>>();

  typename BtcEndorsement::container_t defaultContainer =
      getDefaultContainer<typename BtcEndorsement::container_t>();
  BtcEndorsement expectedEndorsement1 =
      BtcEndorsement::fromContainer(defaultContainer);

  inmemRepo1->put(defaultContainer);

  typename BtcEndorsement::container_t modifiedContainer =
      getModifiedContainer<typename BtcEndorsement::container_t>();
  BtcEndorsement expectedEndorsement2 =
      BtcEndorsement::fromContainer(modifiedContainer);

  inmemRepo1->put(modifiedContainer);

  endorsementRepositoryCopy(*inmemRepo1, *rocksRepo1);

  std::vector<BtcEndorsement> endorsements =
      inmemRepo1->get(expectedEndorsement1.endorsedHash);

  EXPECT_EQ(endorsements.size(), 2);

  if (endorsements[0].id != expectedEndorsement1.id) {
    std::reverse(endorsements.begin(), endorsements.end());
  }

  EXPECT_EQ(expectedEndorsement1.blockOfProof, endorsements[0].blockOfProof);
  EXPECT_EQ(expectedEndorsement1.containingHash,
            endorsements[0].containingHash);
  EXPECT_EQ(expectedEndorsement1.endorsedHash, endorsements[0].endorsedHash);
  EXPECT_EQ(expectedEndorsement1.id, endorsements[0].id);

  EXPECT_EQ(expectedEndorsement2.blockOfProof, endorsements[1].blockOfProof);
  EXPECT_EQ(expectedEndorsement2.containingHash,
            endorsements[1].containingHash);
  EXPECT_EQ(expectedEndorsement2.endorsedHash, endorsements[1].endorsedHash);
  EXPECT_EQ(expectedEndorsement2.id, endorsements[1].id);
}
