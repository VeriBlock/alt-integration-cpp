package api

import (
	"bytes"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestCalculateTopLevelMerkleRoot(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := generateTestPopContext(t, storage)
	defer popContext.popContext.Free()

	index, err := popContext.AltBestBlock()
	assert.NoError(err)

	block, err := index.GetAltBlockHeader()
	assert.NoError(err)

	var popData entities.PopData
	popData.Version = 1

	txRootHash := [veriblock.Sha256HashSize]byte{}

	hash, err := popContext.CalculateTopLevelMerkleRoot(txRootHash, block.PreviousBlock, &popData)
	assert.NoError(err)

	assert.False(bytes.Equal(hash[:], []byte{}))
}

func TestCreateStorageFailure(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage("/tmp/alt-integration")
	defer storage.Free()

	assert.NoError(err)

	storage, err = NewStorage("/tmp/alt-integration")
	assert.Error(err)
	assert.Empty(storage)
}

func TestSaveLoadAllTrees(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage("/tmp/alt-integration")
	assert.NoError(err)

	popContext := generateTestPopContext(t, storage)

	miner := NewMockMiner()

	index, err := miner.MineVbkBlockTip()
	assert.NoError(err)

	vbkBlock, err := index.GetVbkBlockHeader()
	assert.NoError(err)

	state, err := popContext.SubmitVbk(vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)
	assert.NoError(err)

	// generate new block
	newBlock := generateNextAltBlock(&boostrapBlock)

	err = popContext.AcceptBlockHeader(newBlock)
	assert.NoError(err)

	popData, err := popContext.GetPopData()
	assert.NotEqual(popData, nil)
	assert.NoError(err)

	err = popContext.AcceptBlock(newBlock.Hash, popData)
	assert.NoError(err)

	err = popContext.SetState(newBlock.Hash)
	assert.NoError(err)

	err = popContext.SaveAllTrees()
	assert.NoError(err)

	index, err = popContext.AltBestBlock()
	assert.NoError(err)
	assert.NotEqual(index, nil)

	block, err := index.GetAltBlockHeader()
	assert.NoError(err)
	assert.NotEqual(block, nil)

	assert.Equal(block.Hash, newBlock.Hash)
	assert.Equal(block.Height, newBlock.Height)
	assert.Equal(block.PreviousBlock, newBlock.PreviousBlock)
	assert.Equal(block.Timestamp, newBlock.Timestamp)

	// reset state of the popContext
	popContext.popContext.Free()
	popContext = generateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer storage.Free()

	index, err = popContext.AltBestBlock()
	assert.NoError(err)
	assert.NotEqual(index, nil)

	block, err = index.GetAltBlockHeader()
	assert.NoError(err)
	assert.NotEqual(block, nil)

	assert.NotEqual(block.Hash, newBlock.Hash)
	assert.NotEqual(block.Height, newBlock.Height)
	assert.NotEqual(block.PreviousBlock, newBlock.PreviousBlock)
	assert.NotEqual(block.Timestamp, newBlock.Timestamp)
	assert.Equal(block.Hash, newBlock.PreviousBlock)
	assert.Equal(block.Height, newBlock.Height-1)

	err = popContext.LoadAllTrees()
	assert.NoError(err)

	index, err = popContext.AltBestBlock()
	assert.NoError(err)
	assert.NotEqual(index, nil)

	block, err = index.GetAltBlockHeader()
	assert.NoError(err)
	assert.NotEqual(block, nil)

	assert.Equal(block.Hash, newBlock.Hash)
	assert.Equal(block.Height, newBlock.Height)
	assert.Equal(block.PreviousBlock, newBlock.PreviousBlock)
	assert.Equal(block.Timestamp, newBlock.Timestamp)
}
