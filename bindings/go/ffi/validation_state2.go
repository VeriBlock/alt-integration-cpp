// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/validation_state2.h>
import "C"
import (
	"errors"
	"runtime"
)

type ValidationState2 struct {
	ref *C.pop_validation_state_t
}

func NewValidationState2() *ValidationState2 {
	val := &ValidationState2{ref: C.pop_validation_state_new()}
	runtime.SetFinalizer(val, func(v *ValidationState2) {
		v.Free()
	})
	return val
}

func (v *ValidationState2) Free() {
	if v.ref != nil {
		C.pop_validation_state_free(v.ref)
		v.ref = nil
	}
}

func (v *ValidationState2) GetErrorMessage() string {
	if v.ref == nil {
		panic("ValidationState does not initialized")
	}
	str := C.pop_validation_state_get_error_message(v.ref)
	defer freeArrayChar(&str)
	return createString(&str)
}

func (v *ValidationState2) IsValid() bool {
	if v.ref == nil {
		panic("ValidationState does not initialized")
	}
	return bool(C.pop_validation_state_get_is_valid(v.ref))
}

func (v *ValidationState2) IsInvalid() bool {
	if v.ref == nil {
		panic("ValidationState does not initialized")
	}
	return bool(C.pop_validation_state_get_is_invalid(v.ref))
}

// Error ...
func (v *ValidationState2) Error() error {
	if v.IsInvalid() {
		return errors.New(v.GetErrorMessage())
	}
	return nil
}
