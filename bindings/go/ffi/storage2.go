package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/storage2.h>
import "C"
import "runtime"

// Storage2 ...
type Storage2 struct {
	ref *C.pop_storage_t
}

func NewStorage2(path string, state *ValidationState2) *Storage2 {
	val := &Storage2{ref: C.pop_storage_new(C.CString(path), state.ref)}
	runtime.SetFinalizer(val, func(v *Storage2) {
		v.Free()
	})
	return val
}

func (v *Storage2) Free() {
	if v.ref != nil {
		C.pop_storage_free(v.ref)
		v.ref = nil
	}
}