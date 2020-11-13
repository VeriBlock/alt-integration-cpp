package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/mock_miner.h>
import "C"

type MockMiner struct {
	ref *C.MockMiner_t
}

func NewMockMiner() MockMiner { return MockMiner{ref: C.VBK_NewMockMiner()} }

func (v MockMiner) Free() { C.VBK_FreeMockMiner(v.ref) }
