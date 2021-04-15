package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/vtb.h>
import "C"
import "runtime"

type Vtb struct {
	ref *C.pop_vtb_t
}

func (v *AltBlock) Free() {
	if v.ref != nil {
		C.pop_vtb_free(v.ref)
		v.ref = nil
	}
}