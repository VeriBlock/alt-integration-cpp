// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/mock_miner.h>
import "C"
import "runtime"

// MockMiner ...
type MockMiner struct {
	ref   *C.pop_mock_miner_t
	mutex *SafeMutex
}

func (v *MockMiner) validate() {
	v.mutex.AssertMutexLocked("MockMiner is not locked")
	if v.ref == nil {
		panic("MockMiner does not initialized")
	}
}

func NewMockMiner() *MockMiner {
	val := &MockMiner{
		ref:   C.pop_mock_miner_new(),
		mutex: NewSafeMutex(),
	}
	runtime.SetFinalizer(val, func(v *MockMiner) {
		defer v.Lock()()
		v.Free()
	})
	return val
}

func (v *MockMiner) Free() {
	v.mutex.AssertMutexLocked("MockMiner is not locked")
	if v.ref != nil {
		C.pop_mock_miner_free(v.ref)
		v.ref = nil
	}
}

func (v *MockMiner) MineBtcBlockTip() *BtcBlock {
	v.validate()
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlockTip(v.ref))
}

func (v *MockMiner) MineBtcBlock(block *BtcBlock) *BtcBlock {
	v.validate()
	block.validate()
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlock(v.ref, block.ref))
}

func (v *MockMiner) MineVbkBlockTip() *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlockTip(v.ref))
}

func (v *MockMiner) MineVbkBlock(block *VbkBlock) *VbkBlock {
	v.validate()
	block.validate()
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlock(v.ref, block.ref))
}

func (v *MockMiner) MineAtv(pub_data *PublicationData) *Atv {
	v.validate()
	pub_data.validate()
	return createAtv(C.pop_mock_miner_function_mineATV(v.ref, pub_data.ref))
}

func (v *MockMiner) MineVtb(endorsed_block *VbkBlock, last_known_btc_block *BtcBlock) *Vtb {
	v.validate()
	endorsed_block.validate()
	last_known_btc_block.validate()
	return createVtb(C.pop_mock_miner_function_mineVTB(v.ref, endorsed_block.ref, last_known_btc_block.ref))
}

func (v *MockMiner) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
