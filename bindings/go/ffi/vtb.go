// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/vtb.h>
import "C"
import (
	"encoding/json"
	"runtime"

	"github.com/stretchr/testify/assert"
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
	for i := 0; i < len(res); i++ {
		res[i] = createVtb(C.pop_array_vtb_at(array, C.size_t(i)))
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

func (v *Vtb) ToJSON() (map[string]interface{}, error) {
	if v.ref == nil {
		panic("Vtb does not initialized")
	}
	str := C.pop_vtb_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (val1 *Vtb) assertEquals(assert *assert.Assertions, val2 *Vtb) {
	val1.GetContainingBlock().assertEquals(assert, val2.GetContainingBlock())
}
