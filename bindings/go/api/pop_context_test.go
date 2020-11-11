package api

import (
	"bytes"
	"encoding/hex"
	"testing"

	entities "github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/stretchr/testify/assert"
)

func TestPopContext(t *testing.T) {
	assert := assert.New(t)

	config := NewConfig()
	defer config.Free()
	if !config.SelectVbkParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	SetOnGetAltchainID(func() int { return 1 })
	SetOnGetBootstrapBlock(func() string {
		return "000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000000000000009c000005ba"
	})

	popContext := NewPopContext(&config)
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
	vbkblock, _ := hex.DecodeString("41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001")
	stream := bytes.NewReader(vbkblock)
	block := entities.VbkBlock{}
	assert.NoError(block.FromVbkEncoding(stream))
	result := popContext.SubmitVbk(block)
	assert.Equal(2, result)
}
