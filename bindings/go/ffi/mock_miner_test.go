// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import "testing"

func TestMockMinerFree(t *testing.T) {
	miner := NewMockMiner()
	miner.Free()
	miner.Free()
}
