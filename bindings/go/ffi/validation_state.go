package ffi

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -lveriblock-pop-cpp -lstdc++
// #include <veriblock/c/validation_state.h>
import "C"

// ValidationState ...
type ValidationState struct {
	ref *C.VbkValidationState
}

// NewValidationState ...
func NewValidationState() ValidationState {
	return ValidationState{ref: C.VBK_NewValidationState()}
}

// Free ...
func (v *ValidationState) Free() {
	C.VBK_FreeValidationState(v.ref)
}

// GetErrorMessage ...
func (v *ValidationState) GetErrorMessage() string {
	c_str := C.VBK_ValidationState_getErrorMessage(v.ref)
	return C.GoString(c_str)
}

// // IsValid ...
// func (v *ValidationState) IsValid() bool {
// 	res := C.VBK_ValidationState_isValid(v.ref)
// 	return bool(res)
// }

// // IsInvalid ...
// func (v *ValidationState) IsInvalid() bool {
// 	res := C.VBK_ValidationState_isInvalid(v.ref)
// 	return bool(res)
// }
