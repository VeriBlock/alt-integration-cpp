// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

import (
	"encoding/hex"
	"fmt"
	"math/rand"
	"testing"

	"github.com/VeriBlock/alt-integration-cpp/bindings/go/entities"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

var boostrapBlock = entities.AltBlock{
	Height:        1,
	Hash:          []byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
	PreviousBlock: []byte{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	Timestamp:     100,
}

func GenerateTestPopContext(t *testing.T, storage *Storage) *PopContext {
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

	SetOnCheckBlockHeader(func(header []byte, root []byte, state *ffi.ValidationState) bool {
		return true
	})

	SetOnLog(func(log_lvl string, msg string) {
		fmt.Printf("[POP] [%s]\t%s \n", log_lvl, msg)
	})

	return NewPopContext(config, storage, "debug")
}

func generateNextAltBlock(current *entities.AltBlock) *entities.AltBlock {
	var next entities.AltBlock
	next.Hash = make([]byte, len(current.Hash))
	rand.Read(next.Hash)
	next.PreviousBlock = current.Hash
	next.Height = current.Height + 1
	next.Timestamp = current.Timestamp + 1
	return &next
}
