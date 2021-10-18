// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

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

func (v *AltBlock) validate() {
	if v.ref == nil {
		panic("AltBlock does not initialized")
	}
}

func GenerateDefaultAltBlock() *AltBlock {
	return createAltBlock(C.pop_alt_block_generate_default_value())
}

func createAltBlock(ref *C.pop_alt_block_t) *AltBlock {
	val := &AltBlock{ref: ref}
	runtime.SetFinalizer(val, func(v *AltBlock) {
		v.Free()
	})
	return val
}

func NewAltBlock(hash []byte, previousHash []byte, timestamp uint32, height int32) *AltBlock {
	val := &AltBlock{
		ref: C.pop_alt_block_new(createCBytes(hash), createCBytes(previousHash), C.uint32_t(timestamp), C.int32_t(height)),
	}
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
	v.validate()
	array := C.pop_alt_block_get_hash(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *AltBlock) GetPreviousBlock() []byte {
	v.validate()
	array := C.pop_alt_block_get_previous_block(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *AltBlock) GetTimestamp() uint32 {
	v.validate()
	return uint32(C.pop_alt_block_get_timestamp(v.ref))
}

func (v *AltBlock) GetHeight() int32 {
	v.validate()
	return int32(C.pop_alt_block_get_height(v.ref))
}

func (v *AltBlock) ToJSON(reverseHashes bool) (map[string]interface{}, error) {
	v.validate()
	str := C.pop_alt_block_to_json(v.ref, C.bool(reverseHashes))
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *AltBlock) SerializeToVbk() []byte {
	v.validate()
	res := C.pop_alt_block_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func (v *AltBlock) DeserializeFromVbkAltBlock(bytes []byte, config *Config) error {
	state := NewValidationState()
	defer state.Free()

	res := C.pop_alt_block_deserialize_from_vbk(createCBytes(bytes), state.ref, config.ref)
	if res == nil {
		return state.Error()
	}

	v.Free()
	v.ref = res
	return nil
}
