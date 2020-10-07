package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultAltBlockAddon = AltBlockAddon{
		defaultAltPopState,
		[][32]byte{parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000")},
		[][32]byte{parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000")},
		[][12]byte{parse12Bytes("f7de2995898800ab109af967")},
	}
	defaultAltBlockAddonEncoded = "010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000020f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e20f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e18f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000001010cf7de2995898800ab109af967"
)

func TestAltBlockAddonDeserialize(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultAltBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := AltBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultAltBlockAddon.AtvIDs, decoded.AtvIDs)
	assert.Equal(defaultAltBlockAddon.VtbIDs, decoded.VtbIDs)
	assert.Equal(defaultAltBlockAddon.VbkBlockIDs, decoded.VbkBlockIDs)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestAltBlockAddonSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultAltBlockAddon.ToRaw(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultAltBlockAddonEncoded, blockEncoded)
}

func TestAltBlockAddonRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded := parseHex(defaultAltBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := AltBlockAddon{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultAltBlockAddon.AtvIDs, decoded.AtvIDs)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultAltBlockAddonEncoded, blockReEncoded)
}
