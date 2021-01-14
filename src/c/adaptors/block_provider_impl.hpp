// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_ADAPTORS_BLOCK_PROVIDER_IMPL_BINDINGS
#define VERIBLOCK_POP_CPP_ADAPTORS_BLOCK_PROVIDER_IMPL_BINDINGS

#include "veriblock/c/extern.h"
#include "veriblock/storage/block_provider.hpp"

namespace adaptors {

// TODO implement GenericBlockProviderImpl

struct BlockProviderImpl : public altintegration::BlockProvider {
  ~BlockProviderImpl() override = default;

  std::shared_ptr<
      altintegration::details::GenericBlockProvider<altintegration::AltBlock>>
  getAltBlockProvider() const override {
    return nullptr;
  }

  std::shared_ptr<
      altintegration::details::GenericBlockProvider<altintegration::VbkBlock>>
  getVbkBlockProvider() const override {
    return nullptr;
  }

  std::shared_ptr<
      altintegration::details::GenericBlockProvider<altintegration::BtcBlock>>
  getBtcBlockProvider() const override {
    return nullptr;
  }

  /* data */
};

}  // namespace adaptors

#endif