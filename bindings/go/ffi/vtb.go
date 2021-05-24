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

// VtbBlockName ...
const VtbBlockName = "VTB"

type Vtb struct {
	ref *C.pop_vtb_t
}

func (v *Vtb) validate() {
	if v.ref == nil {
		panic("Vtb does not initialized")
	}
}

func generateDefaultVtb() *Vtb {
	return createVtb(C.pop_vtb_generate_default_value())
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

// CreateVtb initializes new Vtb with empty ref. Use DeserializeFromVbk to initialize ref.
func CreateVtb() *Vtb {
	val := &Vtb{ref: nil}
	runtime.SetFinalizer(val, func(v *Vtb) {
		v.Free()
	})
	return val
}

func (v *Vtb) Free() {
	if v.ref != nil {
		C.pop_vtb_free(v.ref)
		v.ref = nil
	}
}

func (v *Vtb) Name() string { return VtbBlockName }

func (v *Vtb) GetID() []byte {
	v.validate()
	array := C.pop_vtb_get_id(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *Vtb) GetContainingBlock() *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_vtb_get_containing_block(v.ref))
}

func (v *Vtb) ToJSON() (map[string]interface{}, error) {
	v.validate()
	str := C.pop_vtb_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *Vtb) SerializeToVbk() []byte {
	v.validate()
	res := C.pop_vtb_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func (v *Vtb) DeserializeFromVbk(bytes []byte) error {
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_vtb_deserialize_from_vbk(createCBytes(bytes), state.ref)
	if res == nil {
		return state.Error()
	}

	v.Free()
	v.ref = res
	return nil
}

func (val1 *Vtb) assertEquals(assert *assert.Assertions, val2 *Vtb) {
	val1.GetContainingBlock().assertEquals(assert, val2.GetContainingBlock())
}
