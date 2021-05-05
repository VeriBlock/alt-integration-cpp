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

func TestPopContext2MemPoolSubmitVbk(t *testing.T) {
	assert := assert.New(t)

	storage, err := NewStorage2(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := generateTestPopContext(t, storage)
	defer context.Free()

	state := NewValidationState2()
	defer state.Free()

	miner := NewMockMiner2()
	defer miner.Free()

	vbk := miner.MineVbkBlockTip()
	defer vbk.Free()

	res := context.MemPoolSubmitVbk(vbk, state)
	assert.Equal(res, 0)
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

// 	res = context.MemPoolSubmitVtb(vtb, state)
// 	t.Log(state.GetErrorMessage())
// 	assert.Equal(res, 0)
// }
