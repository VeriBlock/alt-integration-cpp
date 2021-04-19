// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo CFLAGS: -I../../../include -fsanitize=address
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++ -lrocksdb -ldl -lm
// #include <veriblock/pop/c/validation_state2.h>
import "C"

type ValidationState2 struct {
	ref *C.pop_validation_state_t
}

func (v *ValidationState2) Free() {
	if v.ref != nil {
		C.pop_validation_state_free(v.ref)
		v.ref = nil
	}
}
