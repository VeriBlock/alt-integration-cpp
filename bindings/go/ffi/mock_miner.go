package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/mock_miner.h>
import "C"
import "unsafe"

type MockMiner struct {
	ref *C.MockMiner_t
}

func NewMockMiner() MockMiner { return MockMiner{ref: C.VBK_NewMockMiner()} }

func (v MockMiner) Free() { C.VBK_FreeMockMiner(v.ref) }

func (v MockMiner) MineBtcBlock() []byte {

	blockindexC := (*C.uint8_t)(unsafe.Pointer(nil))
	C.VBK_mineBtcBlockTip(v.ref)

}
