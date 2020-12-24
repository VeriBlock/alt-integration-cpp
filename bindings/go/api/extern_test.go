package api

import (
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestOnAcceptedVBK(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)

	isExecuted1 := false
	isExecuted2 := false

	popContext.AddOnAcceptedVBK(func(*entities.VbkBlock) {
		isExecuted1 = true
	})

	popContext.AddOnAcceptedVBK(func(*entities.VbkBlock) {
		isExecuted2 = true
	})

	miner := NewMockMiner()

	index, err := miner.MineVbkBlockTip()
	assert.NoError(err)

	vbkBlock, err := index.GetVbkBlockHeader()
	assert.NoError(err)

	assert.False(isExecuted1)
	assert.False(isExecuted2)

	state, err := popContext.SubmitVbk(vbkBlock)
	// state == 0, valid vbkBlock
	assert.Equal(0, state)
	assert.NoError(err)

	assert.True(isExecuted1)
	assert.True(isExecuted2)
}

func TestOnAcceptedVTB(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)

	isExecuted1 := false
	isExecuted2 := false

	popContext.AddOnAcceptedVTB(func(*entities.Vtb) {
		isExecuted1 = true
	})

	popContext.AddOnAcceptedVTB(func(*entities.Vtb) {
		isExecuted2 = true
	})

	miner := NewMockMiner()

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

	assert.False(isExecuted1)
	assert.False(isExecuted2)

	state, err = popContext.SubmitVtb(vtb)
	// state == 0, valid vtb
	assert.Equal(0, state)
	assert.NoError(err)

	assert.True(isExecuted1)
	assert.True(isExecuted2)
}

func TestOnAcceptedATV(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)

	isExecuted1 := false
	isExecuted2 := false

	popContext.AddOnAcceptedATV(func(*entities.Atv) {
		isExecuted1 = true
	})

	popContext.AddOnAcceptedATV(func(*entities.Atv) {
		isExecuted2 = true
	})

	miner := NewMockMiner()

	var publicationData entities.PublicationData
	publicationData.ContextInfo = []byte{1, 2, 3, 4}
	publicationData.Header = []byte{1, 2, 3, 4, 5, 7}
	publicationData.Identifier = 1
	publicationData.PayoutInfo = []byte{1, 2, 3, 4, 5, 6}
	atv, err := miner.MineAtv(&publicationData)
	assert.NoError(err)

	assert.False(isExecuted1)
	assert.False(isExecuted2)

	state, err := popContext.SubmitAtv(atv)
	// state == 0, valid atv
	assert.Equal(0, state)
	assert.NoError(err)

	assert.True(isExecuted1)
	assert.True(isExecuted2)
}
