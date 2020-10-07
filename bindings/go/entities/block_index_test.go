package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultBlockIndex = BlockIndex{
		&defaultBtcBlockAddon,
		12345,
		&defaultBtcBlock,
		BlockValidUnknown,
	}
	defaultBlockIndexEncoded = "000030390000002000000000000000b345b7bbf29bda1507a679b97967f99a10ab0088899529def75e16e6cef738a2eba1fe7409318e3f558bec325392427aa3d8eaf46b028654f82213b75c841a011a2e00f29a0000000001060000007b00000084000000d5000000e70000014100000138"
)

func TestBlockIndexDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BlockIndex{}
	decoded.Header = &BtcBlock{}
	decoded.addon = &BtcBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultBlockIndex.Height, decoded.Height)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestBlockIndexSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultBlockIndex.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBlockIndexEncoded, blockEncoded)
}

func TestBlockIndexRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BlockIndex{}
	decoded.Header = &BtcBlock{}
	decoded.addon = &BtcBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultBlockIndex.Height, decoded.Height)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultBlockIndexEncoded, blockReEncoded)
}
