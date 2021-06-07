// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/network_byte_pair.h>
import "C"
import "runtime"

type NetworkBytePair struct {
	ref *C.pop_network_byte_pair_t
}

func (v *NetworkBytePair) validate() {
	if v.ref == nil {
		panic("NetworkBytePair does not initialized")
	}
}

func GenerateDefaultNetworkBytePair() *NetworkBytePair {
	val := &NetworkBytePair{ref: C.pop_network_byte_pair_generate_default_value()}
	runtime.SetFinalizer(val, func(v *NetworkBytePair) {
		v.Free()
	})
	return val
}

func (v *NetworkBytePair) Free() {
	if v.ref != nil {
		C.pop_network_byte_pair_free(v.ref)
		v.ref = nil
	}
}

func (v *NetworkBytePair) HasNetworkByte() bool {
	v.validate()
	return bool(C.pop_network_byte_pair_get_has_network_byte(v.ref))
}

func (v *NetworkBytePair) GetNetworkByte() uint8 {
	v.validate()
	return uint8(C.pop_network_byte_pair_get_network_byte(v.ref))
}

func (v *NetworkBytePair) GetTypeID() uint8 {
	v.validate()
	return uint8(C.pop_network_byte_pair_get_type_id(v.ref))
}
