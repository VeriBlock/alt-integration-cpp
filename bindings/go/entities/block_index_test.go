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

	defaultVbkBlockIndex = BlockIndex{
		&defaultVbkBlockAddon,
		12345,
		&DefaultVbkBlock,
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
