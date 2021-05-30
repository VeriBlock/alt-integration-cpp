// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestValidationStateFree(t *testing.T) {
	t.Parallel()

	state := NewValidationState()

	state.Free()
	state.Free()
}

func TestValidationStateBasic(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	state := NewValidationState()
	defer state.Free()

	assert.Equal(state.IsValid(), true)
	assert.Equal(state.IsInvalid(), false)
	assert.Equal(state.GetErrorMessage(), "")
	assert.NoError(state.Error())

}
