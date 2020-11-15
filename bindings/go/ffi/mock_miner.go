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
func NewMockMiner() MockMiner { return MockMiner{ref: C.VBK_NewMockMiner()} }

// MineBtcBlockTip - Mine new altintegration::BtcBlock on the top of the current btctree.
func (v *MockMiner) MineBtcBlockTip() VbkByteStream {
	return NewVbkByteStream(C.VBK_mineBtcBlockTip(v.ref))
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(tipBlockBytes []byte) VbkByteStream {
	tipBlockBytesC := (*C.uint8_t)(unsafe.Pointer(&tipBlockBytes[0]))
	return NewVbkByteStream(C.VBK_mineBtcBlock(v.ref, tipBlockBytesC, C.int(len(tipBlockBytes))))
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() VbkByteStream {
	return NewVbkByteStream(C.VBK_mineVbkBlockTip(v.ref))
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(tipBlockBytes []byte) VbkByteStream {
	tipBlockBytesC := (*C.uint8_t)(unsafe.Pointer(&tipBlockBytes[0]))
	return NewVbkByteStream(C.VBK_mineVbkBlock(v.ref, tipBlockBytesC, C.int(len(tipBlockBytes))))
}

// Free ...
func (v *MockMiner) Free() { C.VBK_FreeMockMiner(v.ref) }
