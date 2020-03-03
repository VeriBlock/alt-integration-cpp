#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_VBK_ENDORSEMENTS_REPOSITORY_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_VBK_ENDORSEMENTS_REPOSITORY_HPP_

#include <functional>
#include <string>
#include <vector>

#include "veriblock/entities/altblock.hpp"
#include "veriblock/entities/endorsement.hpp"
#include "veriblock/entities/vbkblock.hpp"
#include "veriblock/entities/vbkpoptx.hpp"
#include "veriblock/entities/vbktx.hpp"

namespace VeriBlock {

struct VbkEndorsementRepository {
  using endorsed_hash_t = VbkEndorsement::endorsed_hash_t;
  using containing_hash_t = VbkEndorsement::containing_hash_t;

  virtual ~VbkEndorsementRepository() = default;

  virtual void remove(const endorsed_hash_t& endorsedHash,
                      const endorsed_hash_t& containingHash) = 0;

  virtual void put(const endorsed_hash_t& endorsedHash,
                   const ATV& container) = 0;

  virtual std::vector<VbkEndorsement> get(
      const endorsed_hash_t& endorsedBlockHash) const = 0;
};

}  // namespace VeriBlock

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_BLOCKCHAIN_POP_VBK_ENDORSEMENTS_REPOSITORY_HPP_
