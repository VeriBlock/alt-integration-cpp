package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/validation_state2.h>
import "C"

type ValidationState2 struct {
	ref *C.pop_validation_state_t
}

func (v *ValidationState2) Free() {
	if v.ref != nil {
		C.pop_validation_state_free(v.ref)
		v.ref = nil
	}
}
