// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/validation_state.h>
import "C"
import (
	"errors"
	"runtime"
)

// ValidationState ...
type ValidationState struct {
	ref *C.VbkValidationState
}

// NewValidationState ...
func NewValidationState() *ValidationState {
	state := &ValidationState{ref: C.VBK_NewValidationState()}
	runtime.SetFinalizer(state, func(v *ValidationState) {
		v.Free()
	})
	return state
}

// Free - Dealocates memory allocated for the state.
func (v *ValidationState) Free() {
	if v.ref != nil {
		C.VBK_FreeValidationState(v.ref)
		v.ref = nil
	}
}

// GetErrorMessage ...
func (v *ValidationState) GetErrorMessage() string {
	c_str := C.VBK_ValidationState_getErrorMessage(v.ref)
	return C.GoString(c_str)
}

func (v *ValidationState) Invalid(reject_reason string, debug_reason string) bool {
	res := C.VBK_ValidationState_Invalid(v.ref, C.CString(reject_reason), C.CString(debug_reason))
	return bool(res)
}

// IsValid ...
func (v *ValidationState) IsValid() bool {
	res := C.VBK_ValidationState_isValid(v.ref)
	return bool(res)
}

// IsInvalid ...
func (v *ValidationState) IsInvalid() bool {
	res := C.VBK_ValidationState_isInvalid(v.ref)
	return bool(res)
}

// Error ...
func (v *ValidationState) Error() error {
	if v.IsInvalid() {
		return errors.New(v.GetErrorMessage())
	}
	return nil
}

// Reset ...
func (v *ValidationState) Reset() {
	C.VBK_ValidationState_Reset(v.ref)
}
