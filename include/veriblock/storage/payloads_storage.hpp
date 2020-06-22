// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_

#include <veriblock/blockchain/command_group.hpp>
#include <veriblock/entities/payloads.hpp>
#include <veriblock/entities/vtb.hpp>
#include <veriblock/storage/storage_exceptions.hpp>
#include <veriblock/storage/payloads_base_storage.hpp>

namespace altintegration {

class PayloadsStorage : public PayloadsBaseStorage<AltPayloads>,
                        public PayloadsBaseStorage<VTB> {
 public:
  virtual ~PayloadsStorage() = default;

  PayloadsStorage() {}

  template <typename Payloads>
  Payloads loadPayloads(const typename Payloads::id_t& pid) {
    Payloads payloads;
    bool ret = PayloadsBaseStorage<Payloads>::prepo_->get(pid, &payloads);
    if (!ret) {
      throw StateCorruptedException(
          fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
    }
    return payloads;
  }

  template <typename Payloads>
  void savePayloads(const Payloads& payloads) {
    bool ret = PayloadsBaseStorage<Payloads>::prepo_->put(payloads);
    if (!ret) {
      throw BadIOException(fmt::sprintf("Failed to write payloads id={%s}",
                                        payloads.getId().toHex()));
    }
  }

  template <typename Payloads, typename BlockTree>
  std::vector<CommandGroup> loadCommands(
      const std::vector<typename Payloads::id_t>& pids, BlockTree& tree) {
    std::vector<CommandGroup> out{};
    for (const auto& pid : pids) {
      Payloads payloads;
      if (!PayloadsBaseStorage<Payloads>::prepo_->get(pid, &payloads)) {
        throw StateCorruptedException(
            fmt::sprintf("Failed to read payloads id={%s}", pid.toHex()));
      }
      CommandGroup cg;
      cg.id = pid;
      cg.valid = payloads.valid;
      tree.payloadsToCommands(payloads, cg.commands);
      out.push_back(cg);
    }
    return out;
  }

  template <typename Payloads>
  void setValidity(const typename Payloads::id_t &pid, bool valid) {
    auto payloads = loadPayloads<Payloads>(pid);
    payloads.valid = valid;
    savePayloads(payloads);
  }
};

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_STORAGE_PAYLOADS_STORAGE_HPP_
