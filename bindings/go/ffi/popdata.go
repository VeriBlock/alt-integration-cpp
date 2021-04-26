// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lm
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

func (v *PopData) GetAtvs() []*Atv {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopData) GetVtbs() []*Vtb {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopData) GetContext() []*VbkBlock {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_context(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}
