// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/atv.h>
import "C"
import (
	"encoding/json"
	"runtime"

	"github.com/stretchr/testify/assert"
)

// AtvBlockName ...
const AtvBlockName = "ATV"

type Atv struct {
	ref *C.pop_atv_t
}

func (v *Atv) validate() {
	if v.ref == nil {
		panic("Atv does not initialized")
	}
}

func generateDefaultAtv() *Atv {
	return createAtv(C.pop_atv_generate_default_value())
}

func createAtv(ref *C.pop_atv_t) *Atv {
	val := &Atv{ref: ref}
	runtime.SetFinalizer(val, func(v *Atv) {
		v.Free()
	})
	return val
}

func freeArrayAtv(array *C.pop_array_atv_t) {
	C.pop_array_atv_free(array)
}

func createArrayAtv(array *C.pop_array_atv_t) []*Atv {
	res := make([]*Atv, array.size, array.size)
	for i := 0; i < len(res); i++ {
		res[i] = createAtv(C.pop_array_atv_at(array, C.size_t(i)))
	}
	return res
}

func CreateAtv() *Atv {
	val := &Atv{ref: nil}
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

func (v *Atv) Name() string { return AtvBlockName }

func (v *Atv) GetID() []byte {
	v.validate()
	array := C.pop_atv_get_id(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *Atv) GetBlockOfProof() *VbkBlock {
	v.validate()
	return createVbkBlock(C.pop_atv_get_block_of_proof(v.ref))
}

func (v *Atv) ToJSON() (map[string]interface{}, error) {
	v.validate()
	str := C.pop_atv_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *Atv) SerializeToVbk() []byte {
	v.validate()
	res := C.pop_atv_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func (v *Atv) DeserializeFromVbk(bytes []byte) error {
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_atv_deserialize_from_vbk(createCBytes(bytes), state.ref)
	if res == nil {
		return state.Error()
	}

	v.Free()
	v.ref = res
	return nil
}

func (val1 *Atv) assertEquals(assert *assert.Assertions, val2 *Atv) {
	val1.GetBlockOfProof().assertEquals(assert, val2.GetBlockOfProof())
}
