package api

import (
	"bytes"
	"fmt"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestMineBtcBlock(t *testing.T) {
	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Free()

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
	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Free()

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
	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Free()

	var publication_data entities.PublicationData
	publication_data.ContextInfo = []byte{1, 2, 3, 4}
	publication_data.Header = []byte{1, 2, 3, 4, 5}
	publication_data.Identifier = 10
	publication_data.PayoutInfo = []byte{1, 2, 3, 4, 5, 6}

	atv, err := mockMiner.MineAtv(&publication_data)
	assert.NoError(err)
	assert.True(bytes.Equal(atv.Transaction.PublicationData.ContextInfo, publication_data.ContextInfo))
	assert.True(bytes.Equal(atv.Transaction.PublicationData.Header, publication_data.Header))
	assert.True(bytes.Equal(atv.Transaction.PublicationData.PayoutInfo, publication_data.PayoutInfo))
	assert.Equal(atv.Transaction.PublicationData.Identifier, publication_data.Identifier)
}

func TestMineVtb(t *testing.T) {
	// TODO fix test
	assert := assert.New(t)

	mockMiner := NewMockMiner()
	defer mockMiner.Free()

	index, err := mockMiner.MineVbkBlockTip()
	assert.NoError(err)

	var buffer bytes.Buffer
	index.ToRaw(&buffer)
	var vbkBlock entities.VbkBlock
	err = vbkBlock.FromRaw(&buffer)
	assert.NoError(err)
	// TODO remove this
	fmt.Printf("vbkBlock hash: %v, prev: %v, difficulty: %d, nonce: %d \n", vbkBlock.GetHash(), vbkBlock.PreviousBlock, vbkBlock.Difficulty, vbkBlock.Nonce)

	vtb, err := mockMiner.MineVtb(&vbkBlock)
	assert.NoError(err)
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
