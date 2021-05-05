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
	defer vbk.Free()

	res, err := context.MemPoolSubmitVbk(vbk)
	if err != nil {
		t.Fatal(err)
	}
	assert.Equal(res, 0)

	assert.Equal(len(context.MemPoolGetAtvs()), 0)
	assert.Equal(len(context.MemPoolGetVtbs()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 1)

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
	assert.Equal(len(context.MemPoolGetVtbs()), 0)
	assert.Equal(len(context.MemPoolGetVbkBlocks()), 2)
}

// TODO: popContext.BtcBestBlock()
// func TestPopContext2MemPoolSubmitVtb(t *testing.T) {
// 	assert := assert.New(t)

// 	storage, err := NewStorage2(":inmem:")
// 	assert.NoError(err)
// 	defer storage.Free()

// 	context := generateTestPopContext(t, storage)
// 	defer context.Free()

// 	state := NewValidationState2()
// 	defer state.Free()

// 	miner := NewMockMiner2()
// 	defer miner.Free()

// 	vbk := miner.MineVbkBlockTip()
// 	defer vbk.Free()
// 	res := context.MemPoolSubmitVbk(vbk, state)
// 	assert.Equal(res, 0)

// 	btc := context.BtcBestBlock()
// 	defer btc.Free()

// 	vtb := miner.MineVtb(vbk, btc)
// 	defer vtb.Free()

// 	res, err = context.MemPoolSubmitVtb(vtb)
// 	if err != nil {
//		t.Fatal(err)
//	}
// 	assert.Equal(res, 0)
// }
