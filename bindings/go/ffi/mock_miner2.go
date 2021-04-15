package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/mock_miner2.h>
import "C"
import (
	"runtime"
)

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

func (v *MockMiner2) MineVbkBlockTip() *VbkBlock {
	C.pop_mock_miner_function_mineVbkBlockTip(v.ref)
	return nil
}