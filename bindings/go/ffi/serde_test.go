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
	defaultAltBlockEncoded = "201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c201aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c000005ba0000009c"
	defaultAddressEncoded  = "01166772F51AB208D32771AB1506970EEB664462730B838E"
	defaultAtvEncoded      = "0000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
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

	encodedBytes := assertHexDecode(defaultAltBlockEncoded)

	block, err := DeserializeFromVbkAltBlock(encodedBytes)
	assert.NoError(err)
	assert.NotNil(block)

	assert.Equal(block.GetHeight(), int32(1466))
	assert.Equal(block.GetTimestamp(), uint32(156))
	assert.Equal(block.GetHash(), assertHexDecode("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))
	assert.Equal(block.GetPreviousBlock(), assertHexDecode("1aaaaaaaaaaaa9395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"))

	assert.Equal(block.SerializeToVbk(), encodedBytes)

	block, err = DeserializeFromVbkAltBlock([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(block)
}

func TestAddressSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAddressEncoded)

	address, err := DeserializeFromVbkAddress(encodedBytes)
	assert.NoError(err)
	assert.NotNil(address)

	assert.Equal(address.GetAddress(), "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	assert.Equal(address.GetAddressType(), uint8(1))

	assert.Equal(address.SerializeToVbk(), encodedBytes)

	address, err = DeserializeFromVbkAddress([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(address)
}

func TestAtvSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAtvEncoded)

	atv, err := DeserializeFromVbkAtv(encodedBytes)
	assert.NoError(err)
	assert.NotNil(atv)

	assert.Equal(atv.SerializeToVbk(), encodedBytes)

	atv, err = DeserializeFromVbkAtv([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(atv)
}
