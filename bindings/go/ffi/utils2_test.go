// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"bytes"
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
	endorsedBytes := generateDefaultAltBlock().SerializeToVbk()

	context.GeneratePublicationData(endorsedBytes, txRoot, payoutInfo, popData)

	publicationData, err := context.GeneratePublicationData(endorsedBytes, txRoot, payoutInfo, popData)

	assert.NoError(err)
	assert.NotNil(publicationData)
}

func TestCalculateTopLevelMerkleRoot(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)
	defer context.Free()

	// generate new block
	newBlock := generateNextAltBlock(context.AltGetBootstrapBlock().GetHeader())

	err = context.AcceptBlockHeader(newBlock)
	assert.NoError(err)

	popData := generateDefaultPopData()

	txRootHash := []byte{1, 2, 3, 4}

	hash := context.CalculateTopLevelMerkleRoot(txRootHash, newBlock.GetPreviousBlock(), popData)
	assert.False(bytes.Equal(hash, []byte{}))
}
