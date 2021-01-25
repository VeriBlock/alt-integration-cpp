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

	popContext := generateTestPopContext(t)
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

func TestSaveAllTrees(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.popContext.Free()

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

	popData, err := popContext.GetPop()
	assert.NotEqual(popData, nil)
	assert.NoError(err)

	err = popContext.AcceptBlock(newBlock.Hash, popData)
	assert.NoError(err)

	err = popContext.SetState(newBlock.Hash)
	assert.NoError(err)

	err = popContext.SaveAllTrees()
	assert.NoError(err)
}
