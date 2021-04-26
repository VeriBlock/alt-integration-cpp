// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/storage.h>
import "C"
import "runtime"

type Storage struct {
	ref *C.Storage_t
}

func NewStorage(path string, state *ValidationState) *Storage {
	storage := &Storage{ref: C.VBK_NewStorage(C.CString(path), state.ref)}
	runtime.SetFinalizer(storage, func(v *Storage) {
		v.Free()
	})
	return storage
}

func (v *Storage) Free() {
	if v.ref != nil {
		C.VBK_FreeStorage(v.ref)
		v.ref = nil
	}
}
