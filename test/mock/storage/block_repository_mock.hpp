#ifndef ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_
#define ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_

#include <gmock/gmock.h>

#include <veriblock/storage/block_repository.hpp>

namespace AltIntegrationLib {

template <typename Block>
struct BlockRepository;

template <typename Block>
struct WriteBatchMock : public WriteBatch<Block> {
  ~WriteBatchMock() override = default;
  using stored_block_t = Block;
  using hash_t = typename Block::hash_t;

  MOCK_METHOD1_T(put, void(const stored_block_t& block));
  MOCK_METHOD1_T(removeByHash, void(const hash_t& hash));
  MOCK_METHOD0(clear, void());
  MOCK_METHOD1_T(commit, void(BlockRepository<Block>& repo));
};

template <typename Block>
struct BlockRepositoryMock : public BlockRepository<Block> {
  using stored_block_t = Block;
  using hash_t = typename Block::hash_t;
  using batch_t = WriteBatch<Block>;
  using cursor_t = typename BlockRepository<Block>::cursor_t;

  ~BlockRepositoryMock() override = default;

  MOCK_CONST_METHOD2_T(getByHash, bool(const hash_t&, stored_block_t*));
  MOCK_CONST_METHOD2_T(getManyByHash,
                       size_t(Slice<const hash_t> hashes,
                              std::vector<stored_block_t>* out));
  MOCK_METHOD1_T(put, bool(const stored_block_t& block));
  MOCK_METHOD1_T(removeByHash, bool(const hash_t& hash));
  MOCK_METHOD0(clear, void());
  MOCK_METHOD0_T(newBatch, std::unique_ptr<batch_t>());
  MOCK_METHOD0_T(newCursor, std::shared_ptr<cursor_t>());
};

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_
