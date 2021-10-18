// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"bytes"
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGeneratePublicationData(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)

	context := GenerateTestPopContext(t, storage, config)
	defer context.Lock()()
	defer context.Free()

	popData := GenerateDefaultPopData()
	defer popData.Free()
	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := []byte{1, 2, 3, 4, 5, 6}
	altBlock := GenerateDefaultAltBlock()
	defer altBlock.Free()
	endorsedBytes := altBlock.SerializeToVbk()

	publicationData, err := context.GeneratePublicationData(endorsedBytes, txRoot, payoutInfo, popData)
	defer publicationData.Free()

	assert.NoError(err)
	assert.NotNil(publicationData)
	assert.Equal(payoutInfo, publicationData.GetPayoutInfo())
	assert.Equal(int64(1), publicationData.GetIdentifier())
	assert.NotNil(publicationData.GetHeader())
	assert.NotNil(publicationData.GetContextInfo())
}

func TestCalculateTopLevelMerkleRoot(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := GenerateTestPopContext(t, storage, config)
	defer context.Lock()()
	defer context.Free()

	// generate new block
	newBlock := generateNextAltBlock(context.AltGetBootstrapBlock().GetHeader())

	err = context.AcceptBlockHeader(newBlock)
	assert.NoError(err)

	popData := GenerateDefaultPopData()

	txRootHash := []byte{1, 2, 3, 4}

	hash := context.CalculateTopLevelMerkleRoot(txRootHash, newBlock.GetPreviousBlock(), popData)
	assert.False(bytes.Equal(hash, []byte{}))
}

func TestCheckAll(t *testing.T) {
	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()

	storage, err := NewStorage(":inmem:")
	assert.NoError(err)
	defer storage.Free()

	context := GenerateTestPopContext(t, storage, config)
	defer context.Lock()()
	defer context.Free()

	miner := NewMockMiner()
	defer miner.Lock()()
	defer miner.Free()

	vbkBlock := miner.MineVbkBlockTip()

	// Check VbkBlock
	err = context.CheckVbkBlock(vbkBlock)
	assert.NoError(err)

	vtb := miner.MineVtb(vbkBlock, context.BtcGetBestBlock().GetHeader())

	// Check VTB
	err = context.CheckVtb(vtb)
	assert.NoError(err)

	alt := GenerateDefaultAltBlock()
	payoutInfo := []byte{1, 2, 3, 4, 5, 6}
	txRoot := make([]byte, 32)
	popData := GenerateDefaultPopData()

	pubData, err := context.GeneratePublicationData(alt.SerializeToVbk(), txRoot, payoutInfo, popData)
	assert.NoError(err)
	assert.NotNil(pubData)

	atv := miner.MineAtv(pubData)

	err = context.CheckAtv(atv)
	assert.NoError(err)

	// Failing checks
	err = context.CheckVbkBlock(GenerateDefaultVbkBlock())
	assert.Error(err)

	err = context.CheckVtb(GenerateDefaultVtb())
	assert.Error(err)

	err = context.CheckAtv(GenerateDefaultAtv())
	assert.Error(err)

	err = context.CheckPopData(GenerateDefaultPopData())
	assert.Error(err)
}

func TestSaveLoadAllTrees(t *testing.T) {
	assert := assert.New(t)

	defer os.RemoveAll("/tmp/alt-integration")

	config := NewConfig()
	defer config.Free()

	storage, err := NewStorage("/tmp/alt-integration")
	defer storage.Free()

	assert.NoError(err)

	context := GenerateTestPopContext(t, storage, config)
	unlock := context.Lock()

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

	alt_saved_height := context.AltGetBestBlock().GetHeight()
	vbk_saved_height := context.VbkGetBestBlock().GetHeight()
	btc_saved_height := context.BtcGetBestBlock().GetHeight()

	err = context.SaveAllTrees()
	assert.NoError(err)

	context.Free()
	unlock()
	context = GenerateTestPopContext(t, storage, config)
	defer context.Lock()()
	defer context.Free()

	assert.NotEqual(alt_saved_height, context.AltGetBestBlock().GetHeight())
	assert.NotEqual(vbk_saved_height, context.VbkGetBestBlock().GetHeight())
	assert.NotEqual(btc_saved_height, context.BtcGetBestBlock().GetHeight())

	err = context.LoadAllTrees()
	assert.NoError(err)

	assert.Equal(alt_saved_height, context.AltGetBestBlock().GetHeight())
	assert.Equal(vbk_saved_height, context.VbkGetBestBlock().GetHeight())
	assert.Equal(btc_saved_height, context.BtcGetBestBlock().GetHeight())
}
