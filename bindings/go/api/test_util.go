package api

import (
	"encoding/hex"
	"math/rand"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
)

var boostrapBlock = entities.AltBlock{
	Height:        1,
	Hash:          []byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
	PreviousBlock: []byte{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	Timestamp:     100,
}

func generateTestPopContext(t *testing.T) *PopContext {
	config := NewConfig()
	if !config.SelectVbkParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	SetOnGetAltchainID(func() int64 { return 1 })
	SetOnGetBootstrapBlock(func() string {
		blockBytes, _ := boostrapBlock.ToVbkEncodingBytes()
		return hex.EncodeToString(blockBytes)
	})
	SetOnGetBlockHeaderHash(func(header []byte) []byte {
		var altblock entities.AltBlock
		altblock.FromVbkEncodingBytes(header)
		return altblock.Hash
	})

	SetOnCheckBlockHeader(func(header []byte, root []byte) bool {
		return true
	})

	return NewPopContext(config)
}

func GenerateNextAltBlock(current *entities.AltBlock) (next *entities.AltBlock) {
	// TODO generate random next.Hash
	next.Hash = current.Hash
	next.PreviousBlock = current.Hash
	next.Height = current.Height + 1
	next.Timestamp = current.Timestamp + 1
	rand.Read(next.Hash)
	return next
}
