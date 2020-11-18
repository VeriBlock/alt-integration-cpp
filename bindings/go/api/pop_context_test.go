package api

import (
	"bytes"
	"testing"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestPopContextGetPop(t *testing.T) {
	assert := assert.New(t)

	popContext := generateTestPopContext(t)
	defer popContext.Free()

	res, err := popContext.GetPop()
	assert.NoError(err)
	assert.Equal(
		&entities.PopData{
			Version: 1,
			Context: []entities.VbkBlock{},
			Vtbs:    []entities.Vtb{},
			Atvs:    []entities.Atv{},
		},
		res,
	)
}

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

	result := popContext.SubmitVbk(&vbkBlock)
	// result == 0, valid vbkBlock
	assert.Equal(0, result)
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

	vtb, err := miner.MineVtb(&vbkBlock)
	assert.NoError(err)

	result := popContext.SubmitVtb(vtb)
	// result == 0, valid vtb
	assert.Equal(0, result)
}

func TestPopContextSubmitAtv(t *testing.T) {
	// assert := assert.New(t)

	// popContext := generateTestPopContext(t)
	// defer popContext.Free()

	// miner := NewMockMiner()
	// defer miner.Free()

	// var publication_data entities.PublicationData
	// publication_data.ContextInfo = []byte{1, 2, 3, 4}
	// publication_data.Header = []byte{1, 2, 3, 4, 5}
	// publication_data.Identifier = 1
	// publication_data.PayoutInfo = []byte{1, 2, 3, 4, 5, 6}
	// atv, err := miner.MineAtv(&publication_data)
	// assert.NoError(err)

	// result := popContext.SubmitAtv(atv)
	// // result == 0, valid vbkBlock
	// assert.Equal(0, result)
}
