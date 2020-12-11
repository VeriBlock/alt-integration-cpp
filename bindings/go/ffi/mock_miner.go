package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/mock_miner.h>
import "C"
import "unsafe"

// MockMiner ...
type MockMiner struct {
	ref *C.MockMiner_t
}

// NewMockMiner ...
func NewMockMiner() *MockMiner { return &MockMiner{ref: C.VBK_NewMockMiner()} }

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
func (v *MockMiner) MineAtv(publicationData []byte) (*VbkByteStream, *ValidationState) {
	publicationDataC := (*C.uint8_t)(unsafe.Pointer(&publicationData[0]))
	state := NewValidationState()
	return NewVbkByteStream(C.VBK_MockMiner_mineATV(v.ref, publicationDataC, C.int(len(publicationData)), state.ref)), &state
}

// MineVtb ...
func (v *MockMiner) MineVtb(vbkBlock []byte, hash []byte) (*VbkByteStream, *ValidationState) {
	vbkBlockC := (*C.uint8_t)(unsafe.Pointer(&vbkBlock[0]))
	hashC := (*C.uint8_t)(unsafe.Pointer(&hash[0]))
	state := NewValidationState()
	return NewVbkByteStream(C.VBK_MockMiner_mineVTB(v.ref, vbkBlockC, C.int(len(vbkBlock)), hashC, C.int(len(hash)), state.ref)), &state
}

// Free ...
func (v *MockMiner) Free() { C.VBK_FreeMockMiner(v.ref) }
