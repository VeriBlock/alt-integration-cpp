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
}

func TestMineVbkBlock(t *testing.T) {
	assert := assert.New(t)

	mockMiner := NewMockMiner2()
}