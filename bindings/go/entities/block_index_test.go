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
	defaultBtcBlockIndex = BlockIndex{
		&defaultBtcBlockAddon,
		12345,
		&defaultBtcBlock,
		uint32(BlockBootstrap),
	}
	defaultBtcBlockIndexEncoded = "000030390000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a0000001001060000007b00000084000000d5000000e70000014100000138"

	defaultVbkBlockIndex = BlockIndex{
		&defaultVbkBlockAddon,
		12345,
		&defaultVbkBlock,
		uint32(BlockBootstrap),
	}

	defaultVbkBlockIndexEncoded = "00003039000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f30101000000000000010000001000003085010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"
)

func TestBtcBlockIndexDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := NewBtcBlockIndex()
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultBtcBlockIndex.Height, decoded.Height)
	assert.Equal(defaultBtcBlockIndex.Status, decoded.Status)
	defaultBlock, err := defaultBtcBlockIndex.GetBtcBlockHeader()
	assert.NoError(err)
	decodedBlock, err := decoded.GetBtcBlockHeader()
	assert.NoError(err)
	assert.Equal(defaultBlock.Bits, decodedBlock.Bits)
	assert.Equal(defaultBlock.MerkleRoot, decodedBlock.MerkleRoot)
	assert.Equal(defaultBlock.Nonce, decodedBlock.Nonce)
	assert.Equal(defaultBlock.PreviousBlock, decodedBlock.PreviousBlock)
	assert.Equal(defaultBlock.Timestamp, decodedBlock.Timestamp)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestBtcBlockIndexSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultBtcBlockIndex.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBtcBlockIndexEncoded, blockEncoded)
}

func TestBtcBlockIndexRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := NewBtcBlockIndex()
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultBtcBlockIndex.Height, decoded.Height)
	assert.Equal(defaultBtcBlockIndex.Status, decoded.Status)
	defaultBlock, err := defaultBtcBlockIndex.GetBtcBlockHeader()
	assert.NoError(err)
	decodedBlock, err := decoded.GetBtcBlockHeader()
	assert.NoError(err)
	assert.Equal(defaultBlock.Bits, decodedBlock.Bits)
	assert.Equal(defaultBlock.MerkleRoot, decodedBlock.MerkleRoot)
	assert.Equal(defaultBlock.Nonce, decodedBlock.Nonce)
	assert.Equal(defaultBlock.PreviousBlock, decodedBlock.PreviousBlock)
	assert.Equal(defaultBlock.Timestamp, decodedBlock.Timestamp)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultBtcBlockIndexEncoded, blockReEncoded)
}

func TestVbkBlockIndexDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BlockIndex{}
	decoded.Header = &VbkBlock{}
	decoded.Addon = &VbkBlockAddon{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkBlockIndex.Height, decoded.Height)
	assert.Equal(defaultVbkBlockIndex.Status, decoded.Status)
	defaultBlock, err := defaultVbkBlockIndex.GetVbkBlockHeader()
	assert.NoError(err)
	decodedBlock, err := decoded.GetVbkBlockHeader()
	assert.NoError(err)
	assert.Equal(defaultBlock.Difficulty, decodedBlock.Difficulty)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)
	assert.Equal(defaultBlock.PreviousKeystone, decodedBlock.PreviousKeystone)
	assert.Equal(defaultBlock.SecondPreviousKeystone, decodedBlock.SecondPreviousKeystone)
	assert.Equal(defaultBlock.MerkleRoot, decodedBlock.MerkleRoot)
	assert.Equal(defaultBlock.Nonce, decodedBlock.Nonce)
	assert.Equal(defaultBlock.PreviousBlock, decodedBlock.PreviousBlock)
	assert.Equal(defaultBlock.Timestamp, decodedBlock.Timestamp)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkBlockIndexSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkBlockIndex.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkBlockIndexEncoded, blockEncoded)
}

func TestVbkBlockIndexRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := NewVbkBlockIndex()
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkBlockIndex.Height, decoded.Height)
	assert.Equal(defaultVbkBlockIndex.Status, decoded.Status)
	defaultBlock, err := defaultVbkBlockIndex.GetVbkBlockHeader()
	assert.NoError(err)
	decodedBlock, err := decoded.GetVbkBlockHeader()
	assert.NoError(err)
	assert.Equal(defaultBlock.Difficulty, decodedBlock.Difficulty)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)
	assert.Equal(defaultBlock.PreviousKeystone, decodedBlock.PreviousKeystone)
	assert.Equal(defaultBlock.SecondPreviousKeystone, decodedBlock.SecondPreviousKeystone)
	assert.Equal(defaultBlock.MerkleRoot, decodedBlock.MerkleRoot)
	assert.Equal(defaultBlock.Nonce, decodedBlock.Nonce)
	assert.Equal(defaultBlock.PreviousBlock, decodedBlock.PreviousBlock)
	assert.Equal(defaultBlock.Timestamp, decodedBlock.Timestamp)
	assert.Equal(defaultBlock.Version, decodedBlock.Version)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultVbkBlockIndexEncoded, blockReEncoded)
}

func TestVbkBlockIndexRoundTrip1(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex("00142907001429070002a4b7eba4bb00aeb4d1340cb96f125526d1662b280e99c20cbe8bf8edde4e639e92abf167444730386d504b12c7936037e277040b133b0000ba4680000002040000000101012022b4da7da14606ce232dc4370426a6cb0b8d35127d95e8b1143b4e8209f0184218000000151bc73dcee1332a6f5c53fe6f125526d1662b280e1800000000516bed35dea335872bf4f914e97590366e664887200000000000000001c9bd3eac6a2b97da33e90b50daebc75b906bfd944290e7f401012022b4da7da14606ce232dc4370426a6cb0b8d35127d95e8b1143b4e8209f01842")
	stream := bytes.NewReader(blockEncoded)
	decoded := NewVbkBlockIndex()
	assert.NoError(decoded.FromVbkEncoding(stream))
}

func TestVbkBlockIndexToJSON(t *testing.T) {
	assert := assert.New(t)

	defaultJSON := `{"chainWork":"","containingEndorsements":[],"endorsedBy":[],"header":{"difficulty":16842752,"hash":"2ba076219b4ff7ed36512275cd97599e23096ad42f119b5a","height":5000,"id":"cd97599e23096ad42f119b5a","merkleRoot":"26bbfda7d5e4462ef24ae02d67e47d78","nonce":1,"previousBlock":"449c60619294546ad825af03","previousKeystone":"b0935637860679ddd5","secondPreviousKeystone":"5ee4fd21082e18686e","timestamp":1553699059,"version":2},"height":12345,"ref":12421,"status":16,"stored":{"vtbids":[]}}`
	jsonmap, err := defaultVbkBlockIndex.ToJSON(nil)
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}

func TestBtcBlockIndexToJSON(t *testing.T) {
	assert := assert.New(t)

	defaultJSON := `{"chainWork":"","header":{"bits":436279940,"hash":"ebaa22c5ffd827e96c4450ad5dd35dbec2aa45e15cdb5ce9928f543f4cebf10e","merkleRoot":"f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e","nonce":2599551022,"previousBlock":"f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000","timestamp":1555501858,"version":536870912},"height":12345,"ref":6,"status":16}`
	jsonmap, err := defaultBtcBlockIndex.ToJSON(nil)
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}
