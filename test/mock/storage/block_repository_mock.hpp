#ifndef ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_
#define ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_

#include <gmock/gmock.h>

#include <veriblock/storage/block_repository.hpp>

namespace VeriBlock {

template <typename Block>
struct BlockRepositoryMock : public BlockRepository<Block> {
  using stored_block_t = Block;
  using hash_t = typename Block::hash_t;
  using height_t = typename Block::height_t;
  using batch_t = typename BlockRepository<Block>::WriteBatch;
  using cursor_t = typename BlockRepository<Block>::cursor_t;

  ~BlockRepositoryMock() override = default;

  struct WriteBatchMock : public BlockRepository<Block>::WriteBatch {
    ~WriteBatchMock() override = default;

    MOCK_METHOD1_T(put, bool(const stored_block_t& block));
    MOCK_METHOD1_T(removeByHash, bool(const hash_t& hash));
    MOCK_METHOD1_T(removeByHeight, size_t(height_t height));
    MOCK_METHOD0(clear, void());
  };

  MOCK_CONST_METHOD2_T(getByHash, bool(const hash_t&, stored_block_t*));
  MOCK_CONST_METHOD2_T(getByHeight,
                       bool(height_t, std::vector<stored_block_t>*));
  MOCK_CONST_METHOD2_T(getManyByHash,
                       size_t(Slice<const hash_t> hashes,
                              std::vector<stored_block_t>* out));
  MOCK_METHOD1_T(put, bool(const stored_block_t& block));
  MOCK_METHOD1_T(removeByHash, bool(const hash_t& hash));
  MOCK_METHOD1_T(removeByHeight, size_t(height_t height));
  MOCK_METHOD0_T(newBatch, std::unique_ptr<batch_t>());
  MOCK_METHOD0_T(getCursor, std::shared_ptr<cursor_t>());
  MOCK_METHOD1_T(commit, void(batch_t& batch));
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_TEST_MOCK_STORAGE_BLOCK_REPOSITORY_MOCK_HPP_
