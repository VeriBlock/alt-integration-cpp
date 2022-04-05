// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <list>
#include <veriblock/pop.hpp>

#include "../test/e2e/e2e_utils.hpp"
#include "EntitiesProviders.hpp"
#include "FuzzedDataProvider.hpp"

struct FuzzState {
  altintegration::AltChainParamsRegTest altparam{};
  altintegration::VbkChainParamsRegTest vbkparam{};
  altintegration::BtcChainParamsRegTest btcparam{};

  altintegration::adaptors::InmemStorageImpl storage{};
  altintegration::adaptors::PayloadsStorageImpl payloadsProvider{storage};
  altintegration::adaptors::BlockReaderImpl blockProvider{storage, altparam};

  altintegration::AltBlockTree tree{
      altparam, vbkparam, btcparam, payloadsProvider, blockProvider};
  altintegration::MemPool mempool{tree};

  altintegration::testing_utils::E2EState e2e_state{
      altparam, vbkparam, btcparam};
};

bool handle(FuzzedDataProvider& p, FuzzState& state) {
  if (p.remaining_bytes() == 0) {
    return false;
  }

  // 10% probability - submit action
  // 90% probability - create action
  if (p.ConsumeIntegralInRange<uint32_t>(0, 100) < 10) {
    state.e2e_state.createAction(
        p.ConsumeEnum<altintegration::testing_utils::CreateOption>(),
        p.ConsumeEnum<altintegration::testing_utils::ForkOption>(),
        state.tree);

  } else {
    state.e2e_state.submitAction(
        p.ConsumeEnum<altintegration::testing_utils::SubmitOption>(),
        state.mempool,
        state.tree);
  }

  return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  // make sure input is at least of size 10
  if (Size < 10) {
    return 0;
  }

  FuzzedDataProvider p(Data, Size);
  srand(p.ConsumeIntegral<uint32_t>());
  FuzzState state;

  bool hasEnoughData = true;
  do {
    try {
      hasEnoughData = handle(p, state);
    } catch (const NotEnoughDataException& e) {
      break;
    }
  } while (hasEnoughData);

  return 0;
}