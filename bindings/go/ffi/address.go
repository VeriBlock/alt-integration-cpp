// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/address.h>
import "C"
import (
	"runtime"
)

type Address struct {
	ref *C.pop_address_t
}

func generateDefaultAddress() *Address {
	val := &Address{ref: C.pop_address_generate_default_value()}
	runtime.SetFinalizer(val, func(v *Address) {
		v.Free()
	})
	return val
}

func (v *Address) Free() {
	if v.ref != nil {
		C.pop_address_free(v.ref)
		v.ref = nil
	}
}

func (v *Address) GetAddressType() uint8 {
	if v.ref == nil {
		panic("Address does not initialized")
	}
	return uint8(C.pop_address_get_address_type(v.ref))
}

func (v *Address) GetAddress() string {
	if v.ref == nil {
		panic("Address does not initialized")
	}
	str := C.pop_address_get_address(v.ref)
	defer freeArrayChar(&str)
	return createString(&str)
}
