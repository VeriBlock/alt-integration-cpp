package entities

import (
	"bytes"
	"encoding/hex"
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

	defaultVbkBlockIndex = BlockIndex{&defaultVbkBlockAddon, 12345, &DefaultVbkBlock, BlockValidUnknown}

	defaultVbkBlockIndexEncoded = ""
)

func TestBtcBlockIndexDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockIndexEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BlockIndex{}
	decoded.Header = &BtcBlock{}
	decoded.Addon = &BtcBlockAddon{}
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
	decoded := BlockIndex{}
	decoded.Header = &BtcBlock{}
	decoded.Addon = &BtcBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultBtcBlockIndex.Height, decoded.Height)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultBtcBlockIndexEncoded, blockReEncoded)
}
