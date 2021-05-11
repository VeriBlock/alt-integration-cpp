// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultVbkBlock = VbkBlock{
		5000,
		2,
		parse12Bytes("449c60619294546ad825af03"),
		parse9Bytes("b0935637860679ddd5"),
		parse9Bytes("5ee4fd21082e18686e"),
		parse16Bytes("26bbfda7d5e4462ef24ae02d67e47d78"),
		1553699059,
		16842752,
		1,
	}
	defaultVbkBlockEncoded = "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
)

func parse12Bytes(src string) [12]byte {
	buf := parseHex(src)
	var block [12]byte
	copy(block[:], buf)
	return block
}

func parse9Bytes(src string) [9]byte {
	buf := parseHex(src)
	var block [9]byte
	copy(block[:], buf)
	return block
}

func parse16Bytes(src string) [16]byte {
	buf := parseHex(src)
	var block [16]byte
	copy(block[:], buf)
	return block
}

func TestVbkBlockDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	vbkblock := parseHex(defaultVbkBlockEncoded)
	stream := bytes.NewReader(vbkblock)
	block := VbkBlock{}
	assert.NoError(block.FromVbkEncoding(stream))

	assert.Equal(defaultVbkBlock.Height, block.Height)
	assert.Equal(defaultVbkBlock.Version, block.Version)
	assert.Equal(hex.EncodeToString(defaultVbkBlock.PreviousBlock[:]), hex.EncodeToString(block.PreviousBlock[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.PreviousKeystone[:]), hex.EncodeToString(block.PreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.SecondPreviousKeystone[:]), hex.EncodeToString(block.SecondPreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultVbkBlock.MerkleRoot[:]), hex.EncodeToString(block.MerkleRoot[:]))
	assert.Equal(defaultVbkBlock.Timestamp, block.Timestamp)
	assert.Equal(defaultVbkBlock.Difficulty, block.Difficulty)
	assert.Equal(defaultVbkBlock.Nonce, block.Nonce)
}

func TestVbkBlockSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkBlock.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkBlockEncoded, blockEncoded)
}

func TestVbkBlockRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded, err := hex.DecodeString(defaultVbkBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(blockEncoded)
	decoded := VbkBlock{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkBlock.Version, decoded.Version)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	assert.Equal(defaultVbkBlockEncoded, hex.EncodeToString(outputStream.Bytes()))
}

func TestVbkBlockGetBlockHash(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	block := VbkBlock{}
	block.Height = 5000
	block.Version = 2
	copy(block.PreviousBlock[:], parseHex("94E7DC3E3BE21A96ECCF0FBD"))
	copy(block.PreviousKeystone[:], parseHex("F5F62A3331DC995C36"))
	copy(block.SecondPreviousKeystone[:], parseHex("B0935637860679DDD5"))
	copy(block.MerkleRoot[:], parseHex("DB0F135312B2C27867C9A83EF1B99B98"))
	block.Timestamp = 1553699987
	block.Difficulty = 117586646
	block.Nonce = 1924857207

	hash := block.GetHash()
	assert.Equal("1f45c91342b8ac0ea7ae4d721be2445dc86ddc3f0e454f60", hex.EncodeToString(hash[:]))
}

func TestVbkBlockGetID(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	atvBytes, err := hex.DecodeString(defaultVbkBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(atvBytes)
	vbkblock := VbkBlock{}
	assert.NoError(vbkblock.FromVbkEncoding(stream))
	id := vbkblock.GetID()
	assert.Equal("cd97599e23096ad42f119b5a", hex.EncodeToString(id[:]))
}

func TestVbkToJSON(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	defaultJSON := `{"difficulty":16842752,"hash":"2ba076219b4ff7ed36512275cd97599e23096ad42f119b5a","height":5000,"id":"cd97599e23096ad42f119b5a","merkleRoot":"26bbfda7d5e4462ef24ae02d67e47d78","nonce":1,"previousBlock":"449c60619294546ad825af03","previousKeystone":"b0935637860679ddd5","secondPreviousKeystone":"5ee4fd21082e18686e","timestamp":1553699059,"version":2}`
	jsonmap, err := defaultVbkBlock.ToJSON()
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}
