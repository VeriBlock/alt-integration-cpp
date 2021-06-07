// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/storage.h>
import "C"
import "runtime"

// Storage ...
type Storage struct {
	ref *C.pop_storage_t
}

func (v *Storage) validate() {
	if v.ref == nil {
		panic("Storage does not initialized")
	}
}

func NewStorage(path string) (*Storage, error) {
	state := NewValidationState()
	defer state.Free()

	val := &Storage{ref: C.pop_storage_new(createCString(path), state.ref)}
	runtime.SetFinalizer(val, func(v *Storage) {
		v.Free()
	})
	return val, state.Error()
}

func (v *Storage) Free() {
	if v.ref != nil {
		C.pop_storage_free(v.ref)
		v.ref = nil
	}
}
