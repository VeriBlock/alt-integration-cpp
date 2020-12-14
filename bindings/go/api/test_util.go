package api

import (
	"encoding/hex"
	"math/rand"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
)

func generateTestPopContext(t *testing.T) *PopContext {
	config := NewConfig()
	if !config.SelectVbkParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	SetOnGetAltchainID(func() int { return 1 })
	SetOnGetBootstrapBlock(func() string {
		var block entities.AltBlock
		block.Height = 1
		block.Hash = []byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
		block.PreviousBlock = []byte{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
		block.Timestamp = 100
		blockBytes, _ := block.ToVbkEncodingBytes()
		return hex.EncodeToString(blockBytes)
	})
	SetOnGetBlockHeaderHash(func(header []byte) []byte {
		return header
	})

	return NewPopContext(config)
}

func GenerateNextAltBlock(current *entities.AltBlock) (next *entities.AltBlock) {
	next.Hash = current.Hash
	next.PreviousBlock = current.Hash
	next.Height = current.Height + 1
	next.Timestamp = current.Timestamp + 1
	rand.Read(next.Hash)
	return next
}
