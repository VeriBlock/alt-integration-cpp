// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/storage2.h>
import "C"
import "runtime"

// Storage2 ...
type Storage2 struct {
	ref *C.pop_storage_t
}

func (v *Storage2) validate() {
	if v.ref == nil {
		panic("Storage does not initialized")
	}
}

func NewStorage2(path string) (*Storage2, error) {
	state := NewValidationState2()
	defer state.Free()

	val := &Storage2{ref: C.pop_storage_new(createCString(path), state.ref)}
	runtime.SetFinalizer(val, func(v *Storage2) {
		v.Free()
	})
	return val, state.Error()
}

func (v *Storage2) Free() {
	if v.ref != nil {
		C.pop_storage_free(v.ref)
		v.ref = nil
	}
}
