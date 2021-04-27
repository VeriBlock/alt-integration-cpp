// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/publication_data.h>
import "C"
import "encoding/json"

type PublicationData struct {
	ref *C.pop_publication_data_t
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
