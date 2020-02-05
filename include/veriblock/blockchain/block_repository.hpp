#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_REPOSITORY_HPP_

#include <vector>

namespace VeriBlock {
namespace blockchain {

template <typename Block, typename Id>
struct BlockRepository {
  virtual ~BlockRepository() = default;

  virtual Block getBlock(Id id) = 0;

  virtual size_t size() const = 0;

  virtual
};

}
}


#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_BLOCK_REPOSITORY_HPP_
