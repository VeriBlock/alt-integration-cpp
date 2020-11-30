package api

import (
	"math/rand"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
)

func generateTestPopContext(t *testing.T) PopContext {
	config := NewConfig()
	defer config.Free()
	if !config.SelectVbkParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	if !config.SelectBtcParams("regtest", 0, nil) {
		t.Error("Failed to select btc params")
	}
	SetOnGetAltchainID(func() int { return 1 })
	SetOnGetBootstrapBlock(func() string {
		return "201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c201aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000005ba0000009c"
	})
	SetOnGetBlockHeaderHash(func(header []byte) []byte {
		return header
	})

	return NewPopContext(&config)
}

func GenerateNextAltBlock(current *entities.AltBlock) (next *entities.AltBlock) {
	next.Hash = current.Hash
	next.PreviousBlock = current.Hash
	next.Height = current.Height + 1
	next.Timestamp = current.Timestamp + 1
	rand.Read(next.Hash)
	return next
}
