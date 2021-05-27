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
	defaultPopState = PopState{
		false,
		[]Endorsement{
			&VbkEndorsement{
				parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"),
				parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
				parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
				parse32Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
			},
		},
	}
	defaultAltPopState = PopState{
		true,
		[]Endorsement{
			&AltEndorsement{
				parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"),
				parseHex("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
				parseHex("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
				parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
			},
		},
	}
	defaultPopStateEncoded    = "010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"
	defaultAltPopStateEncoded = "010120f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"
)

func TestPopStateDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(parseHex(defaultPopStateEncoded))
	decoded := PopState{false, nil}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultPopState.ContainingEndorsements, decoded.ContainingEndorsements)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestPopStateSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultPopState.ToRaw(stream))
	encoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultPopStateEncoded, encoded)
}

func TestPopStateRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(parseHex(defaultAltPopStateEncoded))
	decoded := PopState{true, nil}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultAltPopState.IsAlt, decoded.IsAlt)
	assert.Equal(len(defaultAltPopState.ContainingEndorsements), len(decoded.ContainingEndorsements))

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	reEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultAltPopStateEncoded, reEncoded)
}
