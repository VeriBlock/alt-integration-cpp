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
	ref   *C.pop_mock_miner_t
	mutex *SafeMutex
}

func (v *MockMiner2) validate() {
	v.mutex.AssertMutexLocked("MockMiner is not locked")
	if v.ref == nil {
		panic("MockMiner does not initialized")
	}
}

func NewMockMiner2() *MockMiner2 {
	val := &MockMiner2{
		ref:   C.pop_mock_miner_new(),
		mutex: NewSafeMutex(),
	}
	runtime.SetFinalizer(val, func(v *MockMiner2) {
		defer v.Lock()()
		v.Free()
	})
	return val
}

func (v *MockMiner2) Free() {
	v.mutex.AssertMutexLocked("MockMiner is not locked")

	if v.ref != nil {
		C.pop_mock_miner_free(v.ref)
		v.ref = nil
	}
}

func (v *MockMiner2) MineBtcBlockTip() *BtcBlock {
	v.validate()
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlockTip(v.ref))
}

func (v *MockMiner2) MineBtcBlock(block *BtcBlock) *BtcBlock {
	v.validate()
	return createBtcBlock(C.pop_mock_miner_function_mineBtcBlock(v.ref, block.ref))
}

func (v *MockMiner2) MineVbkBlockTip() *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlockTip(v.ref))
}

func (v *MockMiner2) MineVbkBlock(block *VbkBlock) *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_mock_miner_function_mineVbkBlock(v.ref, block.ref))
}

func (v *MockMiner2) MineAtv(pub_data *PublicationData) *Atv {
	v.validate()
	return createAtv(C.pop_mock_miner_function_mineATV(v.ref, pub_data.ref))
}

func (v *MockMiner2) MineVtb(endorsed_block *VbkBlock, last_known_btc_block *BtcBlock) *Vtb {
	v.validate()
	return createVtb(C.pop_mock_miner_function_mineVTB(v.ref, endorsed_block.ref, last_known_btc_block.ref))
}

func (v *MockMiner2) Lock() (unlock func()) {
	v.mutex.Lock()
	return func() {
		v.mutex.Unlock()
	}
}
