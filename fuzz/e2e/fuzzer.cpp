// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "tree.hpp"
#include "veriblock/mock_miner.hpp"
#include "../FuzzedDataProvider.hpp"
#include "../EntitiesProviders.hpp"

static const int hashsize = 4;
static fuzz::Tree TREE;
static altintegration::MockMiner APM;

struct NotEnoughData {};


fuzz::Block ConsumeNextBlock(FuzzedDataProvider& p, fuzz::Block prev) {
  fuzz::Block block;
  block.hash = p.ConsumeBytes<uint8_t>(hashsize);
  block.height = prev.height + 1;
  block.prevhash = prev.hash;
  block.timestamp = prev.timestamp + 1;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  FuzzedDataProvider p(Data, Size);

}