// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"
)

func TestValidationStateFree(t *testing.T) {
	state := NewValidationState()
	state.Free()
	state.Free()
}

// TODO fix memory leaks
// func TestValidationStateBasic(t *testing.T) {
// 	assert := assert.New(t)

// 	state := NewValidationState()
// 	defer state.Free()

// 	assert.NoError(state.Error())
// 	assert.Equal(state.IsInvalid(), false)
// 	assert.Equal(state.IsValid(), true)
// 	assert.Equal(state.GetErrorMessage(), "")

// 	assert.Equal(state.Invalid("error message", "debug message"), false)
// 	assert.Error(state.Error())
// 	assert.Equal(state.IsInvalid(), true)
// 	assert.Equal(state.IsValid(), false)
// 	assert.Equal(state.GetErrorMessage(), "error message, debug message")
// }
