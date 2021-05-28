// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"bytes"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestMineBtcBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Lock()()

	blockTip, err := mockMiner.MineBtcBlockTip()
	assert.NoError(err)
	assert.Equal(uint32(1), blockTip.Height)

	blockTip, err = mockMiner.MineBtcBlock(blockTip.GetHash())
	assert.NoError(err)
	assert.Equal(uint32(2), blockTip.Height)

	blockTip, err = mockMiner.MineBtcBlockTip()
	assert.NoError(err)
	assert.Equal(uint32(3), blockTip.Height)
}

func TestMineVbkBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Lock()()

	blockTip, err := mockMiner.MineVbkBlockTip()
	assert.NoError(err)
	assert.Equal(uint32(1), blockTip.Height)

	blockTip, err = mockMiner.MineVbkBlock(blockTip.GetHash())
	assert.NoError(err)
	assert.Equal(uint32(2), blockTip.Height)

	blockTip, err = mockMiner.MineVbkBlockTip()
	assert.NoError(err)
	assert.Equal(uint32(3), blockTip.Height)
}

func TestMineAtv(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Lock()()

	var publicationData entities.PublicationData
	publicationData.ContextInfo = []byte{1, 2, 3, 4}
	publicationData.Header = []byte{1, 2, 3, 4, 5}
	publicationData.Identifier = 10
	publicationData.PayoutInfo = []byte{1, 2, 3, 4, 5, 6}

	atv, err := mockMiner.MineAtv(&publicationData)
	assert.NoError(err)
	assert.True(bytes.Equal(atv.Transaction.PublicationData.ContextInfo, publicationData.ContextInfo))
	assert.True(bytes.Equal(atv.Transaction.PublicationData.Header, publicationData.Header))
	assert.True(bytes.Equal(atv.Transaction.PublicationData.PayoutInfo, publicationData.PayoutInfo))
	assert.Equal(atv.Transaction.PublicationData.Identifier, publicationData.Identifier)
}

func TestMineVtb(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := GenerateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer popContext.Lock()()

	popContext.BtcBestBlock()

	mockMiner := NewMockMiner()
	defer mockMiner.Lock()()

	index, err := mockMiner.MineVbkBlockTip()
	assert.NoError(err)

	vbkBlock, err := index.GetVbkBlockHeader()
	assert.NoError(err)

	btcTip, err := popContext.BtcBestBlock()
	assert.NoError(err)

	vtb, err := mockMiner.MineVtb(vbkBlock, btcTip.GetHash())
	assert.NoError(err)
	assert.Equal(vtb.ContainingBlock.Height, vbkBlock.Height+1)
	assert.Equal(vtb.Transaction.PublishedBlock.Difficulty, vbkBlock.Difficulty)
	assert.Equal(vtb.Transaction.PublishedBlock.Height, vbkBlock.Height)
	assert.Equal(vtb.Transaction.PublishedBlock.Nonce, vbkBlock.Nonce)
	assert.Equal(vtb.Transaction.PublishedBlock.Timestamp, vbkBlock.Timestamp)
	assert.Equal(vtb.Transaction.PublishedBlock.Version, vbkBlock.Version)
	assert.True(bytes.Equal(vtb.Transaction.PublishedBlock.MerkleRoot[:], vbkBlock.MerkleRoot[:]))
	assert.True(bytes.Equal(vtb.Transaction.PublishedBlock.PreviousBlock[:], vbkBlock.PreviousBlock[:]))
	assert.True(bytes.Equal(vtb.Transaction.PublishedBlock.PreviousKeystone[:], vbkBlock.PreviousKeystone[:]))
	assert.True(bytes.Equal(vtb.Transaction.PublishedBlock.SecondPreviousKeystone[:], vbkBlock.SecondPreviousKeystone[:]))
}
