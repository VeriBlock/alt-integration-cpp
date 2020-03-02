#include <gtest/gtest.h>

#include "veriblock/state_manager.hpp"
#include "veriblock/storage/block_repository_rocks_manager.hpp"

using namespace VeriBlock;

static StateManager<BlockRepositoryRocksManager> state("name");
