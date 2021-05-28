// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

// MemPool tests

func TestPopContextSubmitVbk(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := GenerateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer popContext.Lock()()

	miner := NewMockMiner()
	defer miner.Lock()()

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
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := GenerateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer popContext.Lock()()

	miner := NewMockMiner()
	defer miner.Lock()()

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
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := GenerateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer popContext.Lock()()

	miner := NewMockMiner()
	defer miner.Lock()()

	endorsedBytes, err := boostrapBlock.ToVbkEncodingBytes()
	assert.NoError(err)

	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := [32]byte{}
	popData := entities.PopData{Version: 1}

	publicationData, err := popContext.GeneratePublicationData(endorsedBytes, txRoot, &popData, payoutInfo)
	assert.NoError(err)

	atv, err := miner.MineAtv(publicationData)
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

func TestPopContextAcceptBlock(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	popContext := GenerateTestPopContext(t, storage)
	defer popContext.popContext.Free()
	defer popContext.Lock()()

	miner := NewMockMiner()
	defer miner.Lock()()

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
}
