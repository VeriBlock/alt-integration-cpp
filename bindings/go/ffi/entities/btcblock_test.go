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
