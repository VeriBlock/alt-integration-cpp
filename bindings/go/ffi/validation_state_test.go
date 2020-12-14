package ffi

import "testing"

func TestValidationStateFree(t *testing.T) {
	state := NewValidationState()
	state.Free()
	state.Free()
}
