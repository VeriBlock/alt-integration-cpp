package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/atv.h>
import "C"
import "runtime"

type Atv struct {
	ref *C.pop_atv_t
}

func (v *AltBlock) Free() {
	if v.ref != nil {
		C.pop_atv_free(v.ref)
		v.ref = nil
	}
}
