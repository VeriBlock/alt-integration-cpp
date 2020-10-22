package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultBtcBlockAddon = BtcBlockAddon{
		[]int32{123, 132, 213, 231, 321, 312},
	}
	defaultBtcBlockAddonEncoded = "01060000007b00000084000000d5000000e70000014100000138"
)

func TestBtcBlockAddonDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BtcBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultBtcBlockAddon.Refs, decoded.Refs)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestBtcBlockAddonSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultBtcBlockAddon.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBtcBlockAddonEncoded, blockEncoded)
}

func TestBtcBlockAddonRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultBtcBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := BtcBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultBtcBlockAddon.Refs, decoded.Refs)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultBtcBlockAddonEncoded, blockReEncoded)
}
