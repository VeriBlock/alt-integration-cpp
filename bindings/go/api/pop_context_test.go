// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPopContextFree(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := GenerateTestPopContext(t, storage)
	defer context.Lock()()

	context.Free()
	context.Free()
}

func TestPopContextBlockPrecessing(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := GenerateTestPopContext(t, storage)
	defer context.Lock()()
	defer context.Free()

	// generate new block
	newBlock := generateNextAltBlock(context.AltGetBootstrapBlock().GetHeader())

	err = context.AcceptBlockHeader(newBlock)
	assert.NoError(err)

	miner := NewMockMiner()
	defer miner.Lock()()
	defer miner.Free()

	vbk := miner.MineVbkBlockTip()

	res, err := context.MemPoolSubmitVbk(vbk)
	assert.NoError(err)
	assert.Equal(res, 0)

	vtb := miner.MineVtb(vbk, context.BtcGetBestBlock().GetHeader())

	res, err = context.MemPoolSubmitVtb(vtb)
	assert.NoError(err)
	assert.Equal(res, 0)

	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := make([]byte, 32)
	popData := GenerateDefaultPopData()

	pubData, err := context.GeneratePublicationData(newBlock.SerializeToVbk(), txRoot, payoutInfo, popData)
	assert.NoError(err)
	assert.NotNil(pubData)

	atv := miner.MineAtv(pubData)

	res, err = context.MemPoolSubmitAtv(atv)
	assert.NoError(err)
	assert.Equal(res, 0)

	popData = context.MemPoolGeneratePopData()
	assert.NotNil(popData)

	context.AcceptBlock(newBlock.GetHash(), popData)

	err = context.SetState(newBlock.GetHash())
	assert.NoError(err)

	index := context.AltGetBestBlock()
	assert.NotNil(index)

	assert.Equal(index.GetHeader().GetHash(), newBlock.GetHash())

	context.RemoveSubtree(newBlock.GetHash())

	index, err = context.AltGetBlockIndex(newBlock.GetHash())
	assert.Error(err)
	assert.Nil(index)
}
