package ffi

import "testing"

func TestMockMinerFree(t *testing.T) {
	miner := NewMockMiner()
	miner.Free()
	miner.Free()
}
