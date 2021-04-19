package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/atv.h>
import "C"
import "runtime"

type Atv struct {
	ref *C.pop_atv_t
}

func GenerateDefaultAtv() *Atv {
	val := &Atv{ref: C.pop_atv_generate_default_value()}
	runtime.SetFinalizer(val, func(v *Atv) {
		v.Free()
	})
	return val
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

func (v *Atv) GetBlockOfProof() *VbkBlock {
	if v.ref == nil {
		panic("Vtb does not initialized")
	}
	return createVbkBlock(C.pop_atv_get_block_of_proof(v.ref))
}
