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
	return NewVbkByteStream(C.VBK_MockMiner_mineBtcBlockTip(v.ref))
}

// MineBtcBlock - Mine new altintegration::BtcBlock on the top of the provided block.
func (v *MockMiner) MineBtcBlock(block_hash []byte) VbkByteStream {
	block_hashC := (*C.uint8_t)(unsafe.Pointer(&block_hash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineBtcBlock(v.ref, block_hashC, C.int(len(block_hash))))
}

// MineVbkBlockTip - Mine new altintegration::VbkBlock on the top of the current vbktree.
func (v *MockMiner) MineVbkBlockTip() VbkByteStream {
	return NewVbkByteStream(C.VBK_MockMiner_mineVbkBlockTip(v.ref))
}

// MineVbkBlock - Mine new altintegration::VbkBlock on the top of the provided block.
func (v *MockMiner) MineVbkBlock(block_hash []byte) VbkByteStream {
	block_hashC := (*C.uint8_t)(unsafe.Pointer(&block_hash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineVbkBlock(v.ref, block_hashC, C.int(len(block_hash))))
}

// MineAtv ...
func (v *MockMiner) MineAtv(publication_data []byte) VbkByteStream {
	publication_dataC := (*C.uint8_t)(unsafe.Pointer(&publication_data[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineATV(v.ref, publication_dataC, C.int(len(publication_data))))
}

// MineVtb ...
func (v *MockMiner) MineVtb(vbk_block []byte, hash []byte) VbkByteStream {
	vbk_blockC := (*C.uint8_t)(unsafe.Pointer(&vbk_block[0]))
	hashC := (*C.uint8_t)(unsafe.Pointer(&hash[0]))
	return NewVbkByteStream(C.VBK_MockMiner_mineVTB(v.ref, vbk_blockC, C.int(len(vbk_block)), hashC, C.int(len(hash))))
}

// Free ...
func (v *MockMiner) Free() { C.VBK_FreeMockMiner(v.ref) }
