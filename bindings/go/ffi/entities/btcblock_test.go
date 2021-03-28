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

	btc_block.Free()
	btc_block.Free()
}
