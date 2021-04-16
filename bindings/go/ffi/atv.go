package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/atv.h>
import "C"

type Atv struct {
	ref *C.pop_atv_t
}

func (v *Atv) Free() {
	if v.ref != nil {
		C.pop_atv_free(v.ref)
		v.ref = nil
	}
}

func createAtv(ref *C.pop_atv_t) *Atv {
	val := &Atv{ref: ref}
	runtime.SetFinalizer(val, func(v *Atv) {
		v.Free()
	})
	return val
}
