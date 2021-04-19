package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/entities/vtb.h>
import "C"
import (
	"runtime"
	"unsafe"
)

type Vtb struct {
	ref *C.pop_vtb_t
}

func GenerateDefaultVtb() *Vtb {
	val := &Vtb{ref: C.pop_vtb_generate_default_value()}
	runtime.SetFinalizer(val, func(v *Vtb) {
		v.Free()
	})
	return val
}

func createVtb(ref *C.pop_vtb_t) *Vtb {
	val := &Vtb{ref: ref}
	runtime.SetFinalizer(val, func(v *Vtb) {
		v.Free()
	})
	return val
}

func freeArrayVtb(array *C.pop_array_vtb_t) {
	C.pop_array_vtb_free(array)
}

func createArrayVtb(array *C.pop_array_vtb_t) []*Vtb {
	res := make([]*Vtb, array.size, array.size)
	ptr := uintptr(unsafe.Pointer(array.data))
	for i := 0; i < len(res); i++ {
		val := (*C.pop_vtb_t)(unsafe.Pointer(ptr + unsafe.Sizeof(C.int(0))*uintptr(i)))
		res[i] = createVtb(val)
	}
	return res
}

func (v *Vtb) Free() {
	if v.ref != nil {
		C.pop_vtb_free(v.ref)
		v.ref = nil
	}
}

func (v *Vtb) GetContainingBlock() *VbkBlock {
	if v.ref == nil {
		panic("Vtb does not initialized")
	}
	return createVbkBlock(C.pop_vtb_get_containing_block(v.ref))
}
