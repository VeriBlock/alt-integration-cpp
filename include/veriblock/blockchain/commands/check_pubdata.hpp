// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CHECK_PUBDATA_HPP
#define VERIBLOCK_POP_CPP_CHECK_PUBDATA_HPP

#include <veriblock/blockchain/alt_block_tree.hpp>

namespace altintegration {

struct CheckPublicationData : public Command {
  ~CheckPublicationData() override = default;

  CheckPublicationData(PublicationData pub,
                       AltBlockTree& tree,
                       AltBlock::hash_t endorsed_hash);

  bool Execute(ValidationState& state) override;

  void UnExecute() override {
    // This command does not modify current state, thus UnExecute is a no-op
  }

 private:
  const PublicationData pub;
  const AltBlock::hash_t endorsed_hash;
  AltBlockTree* tree = nullptr;
};

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_CHECK_PUBDATA_HPP
