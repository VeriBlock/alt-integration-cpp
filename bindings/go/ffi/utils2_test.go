// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGeneratePublicationData(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)

	context := generateTestPopContext(t, storage)
	defer context.Free()

	popData := generateDefaultPopData()
	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := []byte{1, 2, 3, 4, 5, 6}
	endorsedBytes := []byte{1, 2, 3, 4, 5, 6}

	context.GeneratePublicationData(endorsedBytes, txRoot, payoutInfo, popData)

	// TODO fix this test
	// publicationData, err := context.GeneratePublicationData(endorsedBytes, txRoot, payoutInfo, popData)

	// assert.NoError(err)
	// assert.NotNil(publicationData)
}
