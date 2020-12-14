package ffi

import "testing"

func TestVbkByteStreamFree(t *testing.T) {
	stream := NewValidationState()
	stream.Free()
	stream.Free()
}
