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
		BlockValidUnknown,
	}
	defaultBtcBlockIndexEncoded = "000030390000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a0000000001060000007b00000084000000d5000000e70000014100000138"

	defaultVbkBlockIndex = BlockIndex{
		&defaultVbkBlockAddon,
		12345,
		&defaultVbkBlock,
		BlockValidUnknown,
	}

	defaultVbkBlockIndexEncoded = "00003039000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f30101000000000000010000000000003085010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e20f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"
)

func TestBtcBlockIndexDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := NewBtcBlockIndex()
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultBtcBlockIndex.Height, decoded.Height)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestBtcBlockIndexSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultBtcBlockIndex.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBtcBlockIndexEncoded, blockEncoded)
}

func TestBtcBlockIndexRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := NewBtcBlockIndex()
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultBtcBlockIndex.Height, decoded.Height)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
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
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultVbkBlockIndex.Height, decoded.Height)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkBlockIndexSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkBlockIndex.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkBlockIndexEncoded, blockEncoded)
}

func TestVbkBlockIndexRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BlockIndex{}
	decoded.Header = &VbkBlock{}
	decoded.Addon = &VbkBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultVbkBlockIndex.Height, decoded.Height)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultVbkBlockIndexEncoded, blockReEncoded)
}

func TestVbkBlockIndexToJSON(t *testing.T) {
	assert := assert.New(t)

	defaultJSON := `{"chainWork":"","containingEndorsements":[],"endorsedBy":[],"header":{"difficulty":16842752,"hash":"2ba076219b4ff7ed36512275cd97599e23096ad42f119b5a","height":5000,"id":"cd97599e23096ad42f119b5a","merkleRoot":"26bbfda7d5e4462ef24ae02d67e47d78","nonce":1,"previousBlock":"449c60619294546ad825af03","previousKeystone":"b0935637860679ddd5","secondPreviousKeystone":"5ee4fd21082e18686e","timestamp":1553699059,"version":2},"height":12345,"ref":12421,"status":0,"stored":{"vtbids":[]}}`
	jsonmap, err := defaultVbkBlockIndex.ToJSON()
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}

func TestBtcBlockIndexToJSON(t *testing.T) {
	assert := assert.New(t)

	defaultJSON := `{"chainWork":"","header":{"bits":436279940,"hash":"ebaa22c5ffd827e96c4450ad5dd35dbec2aa45e15cdb5ce9928f543f4cebf10e","merkleRoot":"f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e","nonce":2599551022,"previousBlock":"f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000","timestamp":1555501858,"version":536870912},"height":12345,"ref":6,"status":0}`
	jsonmap, err := defaultBtcBlockIndex.ToJSON()
	assert.NoError(err)

	res, err := json.Marshal(jsonmap)
	assert.NoError(err)
	assert.Equal(defaultJSON, string(res))
}
