package api

import (
	"bytes"
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

	var buffer bytes.Buffer
	index.Header.ToRaw(&buffer)
	var vbkBlock entities.VbkBlock
	err = vbkBlock.FromRaw(&buffer)
	assert.NoError(err)

	state := popContext.SubmitVbk(&vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)

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

	var buffer bytes.Buffer
	index.Header.ToRaw(&buffer)
	var vbkBlock entities.VbkBlock
	err = vbkBlock.FromRaw(&buffer)
	assert.NoError(err)

	state := popContext.SubmitVbk(&vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)

	btcTip, err := popContext.BtcBestBlock()
	assert.NoError(err)

	vtb, err := miner.MineVtb(&vbkBlock, btcTip.GetHash())
	assert.NoError(err)

	state = popContext.SubmitVtb(vtb)
	// state == 0, valid vtb
	assert.Equal(0, state)

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

	state := popContext.SubmitAtv(atv)
	// state == 0, valid atv
	assert.Equal(0, state)

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
