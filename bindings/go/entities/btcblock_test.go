// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultBtcBlock = BtcBlock{
		536870912,
		parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"),
		parse32Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
		1555501858,
		436279940,
		2599551022,
	}
	defaultBtcBlockEncoded = "0000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a"
)

func parseHex(src string) []byte {
	res, _ := hex.DecodeString(src)
	return res
}

func parse32Bytes(src string) [32]byte {
	buf := parseHex(src)
	var block [32]byte
	copy(block[:], buf)
	return block
}

func TestBtcBlockDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BtcBlock{}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultBtcBlock.Version, decoded.Version)
	assert.Equal(defaultBtcBlock.PreviousBlock, decoded.PreviousBlock)
	assert.Equal(defaultBtcBlock.MerkleRoot, decoded.MerkleRoot)
	assert.Equal(defaultBtcBlock.Timestamp, decoded.Timestamp)
	assert.Equal(defaultBtcBlock.Bits, decoded.Bits)
	assert.Equal(defaultBtcBlock.Nonce, decoded.Nonce)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestBtcBlockSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultBtcBlock.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBtcBlockEncoded, blockEncoded)
}

func TestBtcBlockRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BtcBlock{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultBtcBlock.Version, decoded.Version)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultBtcBlockEncoded, blockReEncoded)
}

func TestBtcBlockGetBlockHash(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockEncoded)
	stream := bytes.NewReader(blockEncoded)
	block := BtcBlock{}
	assert.NoError(block.FromRaw(stream))

	hash := block.GetHash()
	assert.Equal("ebaa22c5ffd827e96c4450ad5dd35dbec2aa45e15cdb5ce9928f543f4cebf10e", hex.EncodeToString(hash[:]))
}
