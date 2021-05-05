// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/publication_data.h>
import "C"
import (
	"encoding/json"
	"runtime"
)

type PublicationData struct {
	ref *C.pop_publication_data_t
}

func createPublicationData(ref *C.pop_publication_data_t) *PublicationData {
	val := &PublicationData{ref: ref}
	runtime.SetFinalizer(val, func(v *PublicationData) {
		v.Free()
	})
	return val
}

func (v *PublicationData) Free() {
	if v.ref != nil {
		C.pop_publication_data_free(v.ref)
		v.ref = nil
	}
}

func (v *PublicationData) ToJSON() (map[string]interface{}, error) {
	if v.ref == nil {
		panic("PublicationData does not initialized")
	}
	str := C.pop_publication_data_to_json(v.ref)
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *PublicationData) SerializeToVbk() []byte {
	if v.ref == nil {
		panic("PublicationData does not initialized")
	}
	res := C.pop_publication_data_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func DeserializeFromVbkPublicationData(bytes []byte) (*PublicationData, error) {
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_publication_data_deserialize_from_vbk(createCBytes(bytes), state.ref)
	if res == nil {
		return nil, state.Error()
	}

	return createPublicationData(res), nil
}
