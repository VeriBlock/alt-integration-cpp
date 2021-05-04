// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestStorageFree(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)

	storage.Free()
	storage.Free()
}

func TestCreateStorageFailure(t *testing.T) {
	assert := assert.New(t)

	defer os.RemoveAll("/tmp/alt-integration")

	storage, err := NewStorage2("/tmp/alt-integration")
	defer storage.Free()

	assert.NoError(err)

	storage, err = NewStorage2("/tmp/alt-integration")
	assert.Error(err)
	assert.Empty(storage)
}
