// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/merkle_path.h>
import "C"
import (
	"runtime"
)

type MerklePath struct {
	ref *C.pop_merkle_path_t
}

func (v *MerklePath) validate() {
	if v.ref == nil {
		panic("MerklePath does not initialized")
	}
}

func GenerateDefaultMerklePath() *MerklePath {
	val := &MerklePath{ref: C.pop_merkle_path_generate_default_value()}
	runtime.SetFinalizer(val, func(v *MerklePath) {
		v.Free()
	})
	return val
}

func (v *MerklePath) Free() {
	if v.ref != nil {
		C.pop_merkle_path_free(v.ref)
		v.ref = nil
	}
}

func (v *MerklePath) GetIndex() int32 {
	v.validate()
	index := C.pop_merkle_path_get_index(v.ref)
	return int32(index)
}

func (v *MerklePath) GetSubject() []byte {
	v.validate()
	array := C.pop_merkle_path_get_subject(v.ref)
	defer freeArrayU8(&array)
	return createBytes(&array)
}

func (v *MerklePath) GetLayers() [][]byte {
	v.validate()
	layers := C.pop_merkle_path_get_layers(v.ref)
	defer freeArrayArrayU8(&layers)
	return createArrayOfArraysU8(&layers)
}
