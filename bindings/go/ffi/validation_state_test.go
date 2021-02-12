package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestValidationStateFree(t *testing.T) {
	state := NewValidationState()
	state.Free()
	state.Free()
}

func TestValidationStateBasic(t *testing.T) {
	assert := assert.New(t)

	state := NewValidationState()
	defer state.Free()

	assert.NoError(state.Error())
	assert.Equal(state.IsInvalid(), false)
	assert.Equal(state.IsValid(), true)
	assert.Equal(state.GetErrorMessage(), "")

	assert.Equal(state.Invalid("error message", "debug message"), false)
	assert.Error(state.Error())
	assert.Equal(state.IsInvalid(), true)
	assert.Equal(state.IsValid(), false)
	assert.Equal(state.GetErrorMessage(), "error message, debug message")
}
