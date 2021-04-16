package ffi

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
	assert.Equal(vbk_block.GetHash(), []byte{0xc8, 0x43, 0x1c, 0x39, 0x10, 0x4b, 0x66, 0x9d, 0x23,
		0x4f, 0x54, 0xb2, 0xa6, 0xb7, 0x6a, 0xdb, 0x58, 0x1c, 0x53, 0x52, 0xca, 0xd8, 0x18, 0x26})

	vbk_block.Free()
	vbk_block.Free()
}

func TestAltBlock(t *testing.T) {
	assert := assert.New(t)

	alt_block := GenerateDefaultAltBlock()

	assert.Equal(alt_block.GetHeight(), int32(1))
	assert.Equal(alt_block.GetTimestamp(), uint32(1))
	assert.Equal(alt_block.GetHash(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})
	assert.Equal(alt_block.GetPreviousBlock(), []byte{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})

	alt_block.Free()
	alt_block.Free()
}

func TestVtb(t *testing.T) {
	assert := assert.New(t)

	vtb := GenerateDefaultVtb()
	vbk_block := GenerateDefaultVbkBlock()

	vtb.GetContainingBlock()

	assert.Equal(vtb.GetContainingBlock().GetHeight(), vbk_block.GetHeight())
	assert.Equal(vtb.GetContainingBlock().GetNonce(), vbk_block.GetNonce())
	assert.Equal(vtb.GetContainingBlock().GetDifficulty(), vbk_block.GetDifficulty())
	assert.Equal(vtb.GetContainingBlock().GetVersion(), vbk_block.GetVersion())
	assert.Equal(vtb.GetContainingBlock().GetTimestamp(), vbk_block.GetTimestamp())
	assert.Equal(vtb.GetContainingBlock().GetMerkleRoot(), vbk_block.GetMerkleRoot())
	assert.Equal(vtb.GetContainingBlock().GetPreviousBlock(), vbk_block.GetPreviousBlock())
	assert.Equal(vtb.GetContainingBlock().GetPreviousKeystone(), vbk_block.GetPreviousKeystone())
	assert.Equal(vtb.GetContainingBlock().GetSecondPreviousKeystone(), vbk_block.GetSecondPreviousKeystone())
	assert.Equal(vtb.GetContainingBlock().GetHash(), vbk_block.GetHash())

	vtb.Free()
	vtb.Free()
}

