// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/mock_miner.h>
import "C"
import (
	"runtime"
	"unsafe"
)

// MockMiner ...
type MockMiner struct {
	ref *C.MockMiner_t
}

// NewMockMiner ...
func NewMockMiner() *MockMiner {
	miner := &MockMiner{ref: C.VBK_NewMockMiner()}
	runtime.SetFinalizer(miner, func(v *MockMiner) {
		miner.Free()
	})
	return miner
}

// Free - Dealocates memory allocated for the miner.
func (v *MockMiner) Free() {
	if v.ref != nil {
		C.VBK_FreeMockMiner(v.ref)
		v.ref = nil
	}
}

// MineBtcBlockTip - Mine new altintegration::BtcBlock on the top of the current btctree.
func (v *MockMiner) MineBtcBlockTip() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MockMiner_mineBtcBlockTip(v.ref))
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(blockHash []byte) *VbkByteStream {
	blockHashC := (*C.uint8_t)(unsafe.Pointer(&blockHash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineBtcBlock(v.ref, blockHashC, C.int(len(blockHash))))
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() *VbkByteStream {
	return NewVbkByteStream(C.VBK_MockMiner_mineVbkBlockTip(v.ref))
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(blockHash []byte) *VbkByteStream {
	blockHashC := (*C.uint8_t)(unsafe.Pointer(&blockHash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineVbkBlock(v.ref, blockHashC, C.int(len(blockHash))))
}

// MineAtv ...
func (v *MockMiner) MineAtv(publicationData []byte, state *ValidationState) *VbkByteStream {
	publicationDataC := (*C.uint8_t)(unsafe.Pointer(&publicationData[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineATV(v.ref, publicationDataC, C.int(len(publicationData)), state.ref))
}

// MineVtb ...
func (v *MockMiner) MineVtb(vbkBlock []byte, hash []byte, state *ValidationState) *VbkByteStream {
	vbkBlockC := (*C.uint8_t)(unsafe.Pointer(&vbkBlock[0]))
	hashC := (*C.uint8_t)(unsafe.Pointer(&hash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineVTB(v.ref, vbkBlockC, C.int(len(vbkBlock)), hashC, C.int(len(hash)), state.ref))
}
