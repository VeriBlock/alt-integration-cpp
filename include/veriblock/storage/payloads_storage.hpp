// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/atv.hpp>
#include <veriblock/entities/popdata.hpp>
#include <veriblock/entities/vbkblock.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/payloads_repository.hpp>
#include <veriblock/storage/payloads_repository_inmem.hpp>
#include <veriblock/storage/storage_exceptions.hpp>

namespace altintegration {

class PayloadsStorage {
 public:
  virtual ~PayloadsStorage() = default;
  PayloadsStorage(std::shared_ptr<PayloadsRepository<ATV>> repoAtv,
                  std::shared_ptr<PayloadsRepository<VTB>> repoVtb,
                  std::shared_ptr<PayloadsRepository<VbkBlock>> repoBlocks)
      : _repoAtv(repoAtv), _repoVtb(repoVtb), _repoBlocks(repoBlocks) {}

  static PayloadsStorage newStorageInmem() {
    std::shared_ptr<PayloadsRepository<ATV>> prepoAtv =
        std::make_shared<PayloadsRepositoryInmem<ATV>>();
    std::shared_ptr<PayloadsRepository<VTB>> prepoVtb =
        std::make_shared<PayloadsRepositoryInmem<VTB>>();
    std::shared_ptr<PayloadsRepository<VbkBlock>> prepoBlocks =
        std::make_shared<PayloadsRepositoryInmem<VbkBlock>>();
    return PayloadsStorage{prepoAtv, prepoVtb, prepoBlocks};
  }

  template <typename Payloads>
  PayloadsRepository<Payloads>& getRepo();

  template <typename Payloads>
  const PayloadsRepository<Payloads>& getRepo() const;

  template <typename Payloads>
  Payloads loadPayloads(const typename Payloads::id_t& pid) {
    Payloads payloads;
    auto& repo = getRepo<Payloads>();
    bool ret = repo.get(pid, &payloads);
    if (!ret) {
      throw StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    return payloads;
  }

  template <typename Payloads>
  void savePayloads(const Payloads& payloads) {
    auto& repo = getRepo<Payloads>();
    repo.put(payloads);
  }

  template <typename Payloads>
  void savePayloadsMany(const std::vector<Payloads>& payloads) {
    auto& repo = getRepo<Payloads>();
    auto batch = repo.newBatch();
    for (const auto& p : payloads) {
      batch->put(p);
    }
    batch->commit();
  }

  void savePayloads(const PopData& pop) {
    savePayloadsMany(pop.context);
    savePayloadsMany(pop.vtbs);
    savePayloadsMany(pop.atvs);
  }

  // realisation in the alt_block_tree, vbK_block_tree
  template <typename BlockTree>
  std::vector<CommandGroup> loadCommands(
      const typename BlockTree::index_t& index, BlockTree& tree);

  template <typename Payloads>
  void setValidity(const typename Payloads::id_t& pid, bool valid) {
    auto payloads = loadPayloads<Payloads>(pid);
    payloads.valid = valid;
    savePayloads(payloads);
  }

  template <typename Payloads>
  bool getValidity(const typename Payloads::id_t& pid) {
    auto payloads = loadPayloads<Payloads>(pid);
    return payloads.valid;
  }

  void setValidity(const CommandGroup& cg, bool valid) {
    if (cg.getPayloadsTypeName() == altintegration::VbkBlock::name()) {
      return setValidity<altintegration::VbkBlock>(
          altintegration::VbkBlock::id_t(cg.id), valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::VTB::name()) {
      return setValidity<altintegration::VTB>(altintegration::VTB::id_t(cg.id),
                                              valid);
    }

    if (cg.getPayloadsTypeName() == altintegration::ATV::name()) {
      return setValidity<altintegration::ATV>(altintegration::ATV::id_t(cg.id),
                                              valid);
    }

    VBK_ASSERT(false && "should not get here");
  }

 protected:
  std::shared_ptr<PayloadsRepository<ATV>> _repoAtv;
  std::shared_ptr<PayloadsRepository<VTB>> _repoVtb;
  std::shared_ptr<PayloadsRepository<VbkBlock>> _repoBlocks;
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_