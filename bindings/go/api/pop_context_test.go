package api

import (
	"testing"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

// MemPool tests

func TestPopContextSubmitVbk(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	miner := NewMockMiner()
	defer miner.Free()

	index, err := miner.MineVbkBlockTip()
	assert.NoError(err)

	vbkBlock, err := index.GetVbkBlockHeader()
	assert.NoError(err)

	state, err := popContext.SubmitVbk(vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)
	assert.NoError(err)

	vtbIDs, err := popContext.GetVtbs()
	assert.NoError(err)
	assert.Equal(0, len(vtbIDs))
	atvIDs, err := popContext.GetAtvs()
	assert.NoError(err)
	assert.Equal(0, len(atvIDs))
	vbkIDs, err := popContext.GetVbkBlocks()
	assert.NoError(err)
	assert.Equal(1, len(vbkIDs))
}

func TestPopContextSubmitVtb(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	miner := NewMockMiner()
	defer miner.Free()

	index, err := miner.MineVbkBlockTip()
	assert.NoError(err)

	vbkBlock, err := index.GetVbkBlockHeader()
	assert.NoError(err)

	state, err := popContext.SubmitVbk(vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)
	assert.NoError(err)

	btcTip, err := popContext.BtcBestBlock()
	assert.NoError(err)

	vtb, err := miner.MineVtb(vbkBlock, btcTip.GetHash())
	assert.NoError(err)

	state, err = popContext.SubmitVtb(vtb)
	// state == 0, valid vtb
	assert.Equal(0, state)
	assert.NoError(err)

	vtbIDs, err := popContext.GetVtbs()
	assert.NoError(err)
	assert.Equal(1, len(vtbIDs))
	atvIDs, err := popContext.GetAtvs()
	assert.NoError(err)
	assert.Equal(0, len(atvIDs))
	vbkIDs, err := popContext.GetVbkBlocks()
	assert.NoError(err)
	assert.Equal(2, len(vbkIDs))
}

func TestPopContextSubmitAtv(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	miner := NewMockMiner()
	defer miner.Free()

	var publicationData entities.PublicationData
	publicationData.ContextInfo = []byte{1, 2, 3, 4}
	publicationData.Header = []byte{1, 2, 3, 4, 5, 7}
	publicationData.Identifier = 1
	publicationData.PayoutInfo = []byte{1, 2, 3, 4, 5, 6}
	atv, err := miner.MineAtv(&publicationData)
	assert.NoError(err)

	state, err := popContext.SubmitAtv(atv)
	// state == 0, valid atv
	assert.Equal(0, state)
	assert.NoError(err)

	vtbIDs, err := popContext.GetVtbs()
	assert.NoError(err)
	assert.Equal(0, len(vtbIDs))
	atvIDs, err := popContext.GetAtvs()
	assert.NoError(err)
	assert.Equal(1, len(atvIDs))
	vbkIDs, err := popContext.GetVbkBlocks()
	assert.NoError(err)
	assert.Equal(1, len(vbkIDs))
}
