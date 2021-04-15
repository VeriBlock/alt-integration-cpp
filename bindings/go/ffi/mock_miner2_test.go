package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)



func TestMockMiner2Free(t *testing.T) {
	mockMiner := NewMockMiner2()
	mockMiner.Free()
	mockMiner.Free()
}


func TestMineBtcBlock(t *testing.T) {
	assert := assert.New(t)

	mockMiner := NewMockMiner2()

	block1 := mockMiner.MineBtcBlockTip()
	block2 := mockMiner.MineBtcBlockTip()
	block3 := mockMiner.MineBtcBlockTip()
	block4 := mockMiner.MineBtcBlockTip()

	assert.Equal(block1.GetHash(), block2.GetPreviousBlock())
	assert.Equal(block2.GetHash(), block3.GetPreviousBlock())
	assert.Equal(block3.GetHash(), block4.GetPreviousBlock())

	block2_1 := mockMiner.MineBtcBlock(block1)
	block3_1 := mockMiner.MineBtcBlock(block2_1)
	block4_1 := mockMiner.MineBtcBlock(block3_1)

	assert.Equal(block1.GetHash(), block2_1.GetPreviousBlock())
	assert.Equal(block2_1.GetHash(), block3_1.GetPreviousBlock())
	assert.Equal(block3_1.GetHash(), block4_1.GetPreviousBlock())

	assert.NotEqual(block2_1.GetHash(), block2.GetHash())
	assert.NotEqual(block3_1.GetHash(), block3.GetHash())
	assert.NotEqual(block4_1.GetHash(), block4.GetHash())
}

func TestMineVbkBlock(t *testing.T) {
	assert := assert.New(t)

	mockMiner := NewMockMiner2()

	block1 := mockMiner.MineVbkBlockTip()
	block2 := mockMiner.MineVbkBlockTip()
	block3 := mockMiner.MineVbkBlockTip()
	block4 := mockMiner.MineVbkBlockTip()

	assert.Equal(block1.GetHeight() + 1, block2.GetHeight())
	assert.Equal(block2.GetHeight() + 1, block3.GetHeight())
	assert.Equal(block3.GetHeight() + 1, block4.GetHeight())


	block2_1 := mockMiner.MineVbkBlock(block1)
	block3_1 := mockMiner.MineVbkBlock(block2_1)
	block4_1 := mockMiner.MineVbkBlock(block3_1)

	assert.Equal(block1.GetHeight() + 1, block2_1.GetHeight())
	assert.Equal(block2_1.GetHeight() + 1, block3_1.GetHeight())
	assert.Equal(block3_1.GetHeight() + 1, block4_1.GetHeight())

	assert.NotEqual(block2_1.GetHash(), block2.GetHash())
	assert.NotEqual(block3_1.GetHash(), block3.GetHash())
	assert.NotEqual(block4_1.GetHash(), block4.GetHash())

}