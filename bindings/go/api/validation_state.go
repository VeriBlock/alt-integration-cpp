// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/validation_state.h>
import "C"
import (
	"errors"
	"runtime"
)

type ValidationState struct {
	ref *C.pop_validation_state_t
}

func (v *ValidationState) validate() {
	if v.ref == nil {
		panic("ValidationState does not initialized")
	}
}

func NewValidationState() *ValidationState {
	val := &ValidationState{ref: C.pop_validation_state_new()}
	runtime.SetFinalizer(val, func(v *ValidationState) {
		v.Free()
	})
	return val
}

func (v *ValidationState) Free() {
	if v.ref != nil {
		C.pop_validation_state_free(v.ref)
		v.ref = nil
	}
}

func (v *ValidationState) GetErrorMessage() string {
	v.validate()
	str := C.pop_validation_state_get_error_message(v.ref)
	defer freeArrayChar(&str)
	return createString(&str)
}

func (v *ValidationState) IsValid() bool {
	v.validate()
	return bool(C.pop_validation_state_get_is_valid(v.ref))
}

func (v *ValidationState) IsInvalid() bool {
	v.validate()
	return bool(C.pop_validation_state_get_is_invalid(v.ref))
}

// Error ...
func (v *ValidationState) Error() error {
	v.validate()
	if v.IsInvalid() {
		return errors.New(v.GetErrorMessage())
	}
	return nil
}
