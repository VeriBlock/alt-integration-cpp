// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/altblock.h>
import "C"
import (
	"encoding/json"
	"runtime"
)

type AltBlock struct {
	ref *C.pop_alt_block_t
}

func generateDefaultAltBlock() *AltBlock {
	return createAltBlock(C.pop_alt_block_generate_default_value())
}

func createAltBlock(ref *C.pop_alt_block_t) *AltBlock {
	val := &AltBlock{ref: ref}
	runtime.SetFinalizer(val, func(v *AltBlock) {
		v.Free()
	})
	return val
}

func (v *AltBlock) Free() {
	if v.ref != nil {
		C.pop_alt_block_free(v.ref)
		v.ref = nil
	}
}

func (v *AltBlock) GetHash() []byte {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
	array := C.pop_alt_block_get_hash(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *AltBlock) GetPreviousBlock() []byte {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
	array := C.pop_alt_block_get_previous_block(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *AltBlock) GetTimestamp() uint32 {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
	return uint32(C.pop_alt_block_get_timestamp(v.ref))
}

func (v *AltBlock) GetHeight() int32 {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
	return int32(C.pop_alt_block_get_height(v.ref))
}

func (v *AltBlock) ToJSON(reverseHashes bool) (map[string]interface{}, error) {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
	str := C.pop_alt_block_to_json(v.ref, C.bool(reverseHashes))
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *AltBlock) SerializeToVbk() []byte {
	res := C.pop_alt_block_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func DeserializeFromVbkAltBlock(bytes []byte) (*AltBlock, error) {
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_alt_block_deserialize_from_vbk(createCBytes(bytes), state.ref)
	if res == nil {
		return nil, state.Error()
	}

	return createAltBlock(res), nil
}
