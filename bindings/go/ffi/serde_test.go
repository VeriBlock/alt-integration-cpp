// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultAltBlockDecoded = "201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c201aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000005ba0000009c"
	defaultAddressDecoded  = "01166772F51AB208D32771AB1506970EEB664462730B838E"
)

func assertHexDecode(str string) []byte {
	res, err := hex.DecodeString(str)
	if err != nil {
		panic(err)
	}
	return res
}

func TestAltBlockSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAltBlockDecoded)

	block, err := DeserializeFromVbkAltBlock(encodedBytes)
	assert.NoError(err)
	assert.NotNil(block)

	assert.Equal(block.GetHeight(), int32(1466))
	assert.Equal(block.GetTimestamp(), uint32(156))
	assert.Equal(block.GetHash(), assertHexDecode("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))
	assert.Equal(block.GetPreviousBlock(), assertHexDecode("1aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))

	assert.Equal(block.SerializeToVbk(), encodedBytes)
}

func TestAddressSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAddressDecoded)

	address, err := DeserializeFromVbkAddress(encodedBytes)
	assert.NoError(err)
	assert.NotNil(address)

	assert.Equal(address.GetAddress(), "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	assert.Equal(address.GetAddressType(), uint8(1))

	assert.Equal(address.SerializeToVbk(), encodedBytes)
}
