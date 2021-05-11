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
	defaultVbkBlockAddon = VbkBlockAddon{
		defaultPopState,
		12421,
		[]VtbID{parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000")},
	}
	defaultVbkBlockAddonEncoded = "00003085010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"
)

func TestVbkBlockAddonDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := VbkBlockAddon{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkBlockAddon.VtbIDs, decoded.VtbIDs)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkBlockAddonSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkBlockAddon.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkBlockAddonEncoded, blockEncoded)
}

func TestVbkBlockAddonRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkBlockAddonEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := VbkBlockAddon{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkBlockAddon.VtbIDs, decoded.VtbIDs)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultVbkBlockAddonEncoded, blockReEncoded)
}
