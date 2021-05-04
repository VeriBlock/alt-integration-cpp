// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/mock_miner2.h>
import "C"
import "runtime"

// MockMiner ...
type MockMiner2 struct {
	ref *C.pop_mock_miner_t
}

func NewMockMiner2() *MockMiner2 {
	val := &MockMiner2{ref: C.pop_mock_miner_new()}
	runtime.SetFinalizer(val, func(v *MockMiner2) {
		v.Free()
	})
	return val
}

func (v *MockMiner2) Free() {
	if v.ref != nil {
		C.pop_mock_miner_free(v.ref)
		v.ref = nil
	}
}

func (v *MockMiner2) MineBtcBlockTip() *BtcBlock {
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlockTip(v.ref))
}

func (v *MockMiner2) MineBtcBlock(block *BtcBlock) *BtcBlock {
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlock(v.ref, block.ref))
}

func (v *MockMiner2) MineVbkBlockTip() *VbkBlock {
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlockTip(v.ref))
}

func (v *MockMiner2) MineVbkBlock(block *VbkBlock) *VbkBlock {
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlock(v.ref, block.ref))
}

func (v *MockMiner2) MineAtv(pub_data *PublicationData) *Atv {
	return createAtv(C.pop_mock_miner_function_mineATV(v.ref, pub_data.ref))
}

func (v *MockMiner2) MineVtb(endorsed_block *VbkBlock, last_known_btc_block *BtcBlock) *Vtb {
	return createVtb(C.pop_mock_miner_function_mineVTB(v.ref, endorsed_block.ref, last_known_btc_block.ref))
}
