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
	defaultVbkBlockEncoded = "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
	defaultBtcBlockEncoded = "500000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a"
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

func TestVbkBlockSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultVbkBlockEncoded)

	vbkBlock, err := DeserializeFromVbkVbkBlock(encodedBytes)
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	assert.Equal(vbkBlock.GetHeight(), int32(5000))
	assert.Equal(vbkBlock.GetVersion(), int16(2))
	assert.Equal(vbkBlock.GetDifficulty(), int32(16842752))
	assert.Equal(vbkBlock.GetNonce(), uint64(1))
	assert.Equal(vbkBlock.GetTimestamp(), uint32(1553699059))
	assert.Equal(vbkBlock.GetPreviousBlock(), assertHexDecode("449c60619294546ad825af03"))
	assert.Equal(vbkBlock.GetPreviousKeystone(), assertHexDecode("b0935637860679ddd5"))
	assert.Equal(vbkBlock.GetSecondPreviousKeystone(), assertHexDecode("5ee4fd21082e18686e"))
	assert.Equal(vbkBlock.GetMerkleRoot(), assertHexDecode("26bbfda7d5e4462ef24ae02d67e47d78"))

	assert.Equal(vbkBlock.SerializeToVbk(), encodedBytes)

	vbkBlock, err = DeserializeFromVbkVbkBlock([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(vbkBlock)
}

func TestBtcBlockSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultBtcBlockEncoded)

	btcBlock, err := DeserializeFromVbkBtcBlock(encodedBytes)
	assert.NoError(err)
	assert.NotNil(btcBlock)

	assert.Equal(btcBlock.GetVersion(), uint32(536870912))
	assert.Equal(btcBlock.GetTimestamp(), uint32(1555501858))
	assert.Equal(btcBlock.GetDifficulty(), uint32(436279940))
	assert.Equal(btcBlock.GetNonce(), uint32(0x9af2002e))
	assert.Equal(btcBlock.GetPreviousBlock(), assertHexDecode("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"))
	assert.Equal(btcBlock.GetMerkleRoot(), assertHexDecode("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"))

	assert.Equal(btcBlock.SerializeToVbk(), encodedBytes)

	btcBlock, err = DeserializeFromVbkBtcBlock([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(btcBlock)
}

func TestAtvSerde(t *testing.T) {
	assert := assert.New(t)

	encodedBytes := assertHexDecode(defaultAtvEncoded)

	vbkBlock, err := DeserializeFromVbkVbkBlock(assertHexDecode(defaultVbkBlockEncoded))
	assert.NoError(err)
	assert.NotNil(vbkBlock)

	atv, err := DeserializeFromVbkAtv(encodedBytes)
	assert.NoError(err)
	assert.NotNil(atv)

	atv.GetBlockOfProof().assertEquals(assert, vbkBlock)

	assert.Equal(atv.SerializeToVbk(), encodedBytes)

	atv, err = DeserializeFromVbkAtv([]byte{1, 2, 3, 4})
	assert.Error(err)
	assert.Nil(atv)
}
