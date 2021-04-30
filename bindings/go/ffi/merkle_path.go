// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/merkle_path.h>
import "C"
import (
	"math/big"
	"runtime"
)

type MerklePath struct {
	ref *C.pop_merkle_path_t
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
	if v.ref == nil {
		panic("MerklePath does not initialized")
	}
	index := C.pop_merkle_path_get_index(v.ref)
	return int32(index)
}

func (v *MerklePath) GetSubject() *big.Int {
	if v.ref == nil {
		panic("MerklePath does not initialized")
	}
	array := C.pop_merkle_path_get_subject(v.ref)
	defer freeArrayU8(&array)
	bytes := createBytes(&array)
	subject := new(big.Int).SetBytes(bytes)
	return subject
}

func (v *MerklePath) GetLayers() []*big.Int {
	if v.ref == nil {
		panic("MerklePath does not initialized")
	}

	res := make([]*big.Int, 0)
	array := C.pop_merkle_path_get_layers(v.ref)
	defer freeArrayArrayU8(&array)
	return res
}
