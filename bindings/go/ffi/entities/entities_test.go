package entities

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestBtcBlock(t *testing.T) {
	assert := assert.New(t)

	btc_block := GenerateDefaultBtcBlock()

	assert.Equal(btc_block.GetDifficulty(), uint32(1))
	assert.Equal(btc_block.GetNonce(), uint32(1))
	assert.Equal(btc_block.GetTimestamp(), uint32(1))
	assert.Equal(btc_block.GetVersion(), uint32(1))
	assert.Equal(btc_block.GetMerkleRoot(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(btc_block.GetPreviousBlock(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(btc_block.GetHash(), []byte{0x16, 0x4a, 0x73, 0xce, 0x49, 0xbb, 0x95, 0xdf, 0x8f, 0x54, 0x71, 0x69, 0xd5, 0x73, 0x1f, 0x06,
		0x9a, 0x83, 0x71, 0x4b, 0xb4, 0x31, 0xaf, 0xa0, 0x9a, 0x24, 0xe0, 0x88, 0xc9, 0xff, 0x05, 0x1c})

	btc_block.Free()
	btc_block.Free()
}

func TestVbkBlock(t *testing.T) {
	assert := assert.New(t)

	vbk_block := GenerateDefaultVbkBlock()

	assert.Equal(vbk_block.GetHeight(), int32(1))
	assert.Equal(vbk_block.GetNonce(), uint64(1))
	assert.Equal(vbk_block.GetDifficulty(), int32(1))
	assert.Equal(vbk_block.GetVersion(), int16(1))
	assert.Equal(vbk_block.GetTimestamp(), uint32(1))
	assert.Equal(vbk_block.GetMerkleRoot(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetPreviousBlock(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetPreviousKeystone(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(vbk_block.GetSecondPreviousKeystone(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1})

	vbk_block.Free()
	vbk_block.Free()
}
