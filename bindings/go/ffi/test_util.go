// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"crypto/rand"
	"fmt"
	"testing"
)

func GenerateTestPopContext(t *testing.T, storage *Storage2) *PopContext2 {
	config := NewConfig2()
	defer config.Free()

	config.SelectVbkParams("regtest", 0, "")
	config.SelectBtcParams("regtest", 0, "")

	SetOnGetAltchainID(func() int64 { return 1 })
	SetOnGetBootstrapBlock(func() AltBlock {
		return *generateDefaultAltBlock()
	})
	SetOnGetBlockHeaderHash(func(header []byte) []byte {
		altblock := NewAltBlock([]byte{}, []byte{}, 0, 0)
		err := altblock.DeserializeFromVbkAltBlock(header)
		if err != nil {
			panic(err)
		}
		return altblock.GetHash()
	})

	SetOnCheckBlockHeader(func(header []byte, root []byte) bool {
		return true
	})

	SetOnLog(func(log_lvl string, msg string) {
		fmt.Printf("[POP] [%s]\t%s \n", log_lvl, msg)
	})

	return NewPopContext2(config, storage, "debug")
}

func generateNextAltBlock(current *AltBlock) *AltBlock {
	currentHash := current.GetHash()

	nextHash := make([]byte, len(currentHash))
	rand.Read(nextHash)

	return NewAltBlock(nextHash, currentHash, current.GetTimestamp()+1, current.GetHeight()+1)
}
