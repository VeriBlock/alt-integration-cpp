// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPopContext2Free(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)

	context.Free()
	context.Free()
}

func TestPopContext2AltBlockTreeAcceptBlockHeader(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)
	defer context.Free()

	alt := generateDefaultAltBlock()
	defer alt.Free()

	res, err := context.AltBlockTreeAcceptBlockHeader(alt)
	if err != nil {
		t.Fatal(err)
	}
	assert.Equal(res, true)
}

func TestPopContext2MemPoolSubmitAll(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)
	defer context.Free()

	assert.Equal(len(context.MemPoolGetAtvs()), 0)
	assert.Equal(len(context.MemPoolGetVtbs()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 0)

	miner := NewMockMiner2()
	defer miner.Free()

	vbk := miner.MineVbkBlockTip()

	res, err := context.MemPoolSubmitVbk(vbk)
	assert.NoError(err)
	assert.Equal(res, 0)

	assert.Equal(len(context.MemPoolGetAtvs()), 0)
	assert.Equal(len(context.MemPoolGetVtbs()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 1)

	vtb := miner.MineVtb(vbk, context.BtcGetBestBlock().GetHeader())

	res, err = context.MemPoolSubmitVtb(vtb)
	assert.NoError(err)
	assert.Equal(res, 0)

	assert.Equal(len(context.MemPoolGetAtvs()), 0)
	assert.Equal(len(context.MemPoolGetVtbs()), 1)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 2)

	alt := generateDefaultAltBlock()
	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := make([]byte, 32)
	popData := generateDefaultPopData()

	pubData, err := context.GeneratePublicationData(alt.SerializeToVbk(), txRoot, payoutInfo, popData)
	assert.NoError(err)
	assert.NotNil(pubData)

	atv := miner.MineAtv(pubData)

	res, err = context.MemPoolSubmitAtv(atv)
	assert.NoError(err)
	assert.Equal(res, 0)

	assert.Equal(len(context.MemPoolGetAtvs()), 1)
	assert.Equal(len(context.MemPoolGetVtbs()), 1)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 3)
}

func TestPopContext2MemPoolSubmitStatefullFailed(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)
	defer context.Free()

	assert.Equal(len(context.MemPoolGetAtvsInFlight()), 0)
	assert.Equal(len(context.MemPoolGetVtbsInFlight()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocksInFlight()), 0)

	miner := NewMockMiner2()
	defer miner.Free()

	// Make a context gap for the vbk blocks
	miner.MineVbkBlockTip()
	vbk := miner.MineVbkBlockTip()

	res, err := context.MemPoolSubmitVbk(vbk)
	assert.Error(err)
	assert.Equal(res, 1)

	assert.Equal(len(context.MemPoolGetAtvsInFlight()), 0)
	assert.Equal(len(context.MemPoolGetVtbsInFlight()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocksInFlight()), 1)

	vtb := miner.MineVtb(vbk, context.BtcGetBestBlock().GetHeader())

	res, err = context.MemPoolSubmitVtb(vtb)
	assert.Error(err)
	assert.Equal(res, 1)

	assert.Equal(len(context.MemPoolGetAtvsInFlight()), 0)
	assert.Equal(len(context.MemPoolGetVtbsInFlight()), 1)
	assert.Equal(len(context.MemPoolGetVbkBlocksInFlight()), 1)

	alt := generateDefaultAltBlock()
	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := make([]byte, 32)
	popData := generateDefaultPopData()

	pubData, err := context.GeneratePublicationData(alt.SerializeToVbk(), txRoot, payoutInfo, popData)
	assert.NoError(err)
	assert.NotNil(pubData)

	atv := miner.MineAtv(pubData)

	res, err = context.MemPoolSubmitAtv(atv)
	assert.Error(err)
	assert.Equal(res, 1)

	assert.Equal(len(context.MemPoolGetAtvsInFlight()), 1)
	assert.Equal(len(context.MemPoolGetVtbsInFlight()), 1)
	assert.Equal(len(context.MemPoolGetVbkBlocksInFlight()), 1)
}
