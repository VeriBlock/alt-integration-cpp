package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/popdata.h>
import "C"
import "runtime"

type PopData struct {
	ref *C.pop_pop_data_t
}

func GenerateDefaultPopData() *PopData {
	val := &PopData{ref: C.pop_pop_data_generate_default_value()}
	runtime.SetFinalizer(val, func(v *PopData) {
		v.Free()
	})
	return val
}

func (v *PopData) Free() {
	if v.ref != nil {
		C.pop_pop_data_free(v.ref)
		v.ref = nil
	}
}
