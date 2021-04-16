package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/vtb.h>
import "C"

type Vtb struct {
	ref *C.pop_vtb_t
}

func (v *Vtb) Free() {
	if v.ref != nil {
		C.pop_vtb_free(v.ref)
		v.ref = nil
	}
}

func createVtb(ref *C.pop_vtb_t) *Vtb {
	val := &Vtb{ref: ref}
	runtime.SetFinalizer(val, func(v *Vtb) {
		v.Free()
	})
	return val
}
