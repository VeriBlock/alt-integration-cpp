package api

import (
	"testing"
)

func TestMockMiner(t *testing.T) {
	// assert := assert.New(t)

	// config := NewConfig()
	// defer config.Free()
	// if !config.SelectVbkParams("regtest", 0, nil) {
	// 	t.Error("Failed to select btc params")
	// }
	// if !config.SelectBtcParams("regtest", 0, nil) {
	// 	t.Error("Failed to select btc params")
	// }
	// SetOnGetAltchainID(func() int { return 1 })
	// SetOnGetBootstrapBlock(func() string {
	// 	return "000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000000000000009c000005ba"
	// })

	// popContext := NewPopContext(&config)
	// defer popContext.Free()

	// mockMiner := NewMockMiner()
	// defer mockMiner.Free()

	// blockTip, err := mockMiner.MineVbkBlockTip()
	// assert.NoError(err)
	// assert.Equal(uint32(1), blockTip.Height)

	// vbkBlock, err := mockMiner.MineVbkBlock(blockTip)
	// assert.NoError(err)
	// assert.Equal(uint32(2), vbkBlock.Height)

	// block := vbkBlock.Header.(*entities.VbkBlock)
	// result := popContext.SubmitVbk(block)
	// assert.Equal(2, result)

	// vbks, err := popContext.GetVbkBlocks()
	// assert.NoError(err)
	// assert.Equal(0, len(vbks))
}
