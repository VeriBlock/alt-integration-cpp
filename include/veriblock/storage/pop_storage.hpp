#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_

#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/block_repository.hpp>
#include <veriblock/storage/block_repository_inmem.hpp>
#include <veriblock/storage/endorsement_storage.hpp>

namespace altintegration {

class PopStorage {
 public:
  virtual ~PopStorage() = default;
  PopStorage()
      : repoVbk_(std::move(std::make_shared<BlockRepositoryInmem<VbkBlock>>())),
        repoBtc_(std::move(std::make_shared<BlockRepositoryInmem<BtcBlock>>())),
        endorsementsAtv_(
            std::move(std::make_shared<EndorsementStorage<AltPayloads, AltTree>>())),
        endorsementsVtb_(
            std::move(std::make_shared<EndorsementStorage<VTB, VbkBlockTree>>())) {}

  BlockRepository<VbkBlock>& vbkIndex() { return *repoVbk_; }
  const BlockRepository<VbkBlock>& vbkIndex() const { return *repoVbk_; }

 private:
  std::shared_ptr<BlockRepository<VbkBlock>> repoVbk_;
  std::shared_ptr<BlockRepository<BtcBlock>> repoBtc_;
  std::shared_ptr<EndorsementStorage<AltPayloads, AltTree>> endorsementsAtv_;
  std::shared_ptr<EndorsementStorage<VTB, VbkBlockTree>> endorsementsVtb_;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_POP_STORAGE_HPP_