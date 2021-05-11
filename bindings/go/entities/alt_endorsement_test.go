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
	defaultAltEndorsement = AltEndorsement{
		parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"),
		parseHex("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
		parseHex("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
		parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
	}
	defaultAltEndorsementEncoded = "20f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000020f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e20f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e18f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"
)

func parse24Bytes(src string) [24]byte {
	buf := parseHex(src)
	var block [24]byte
	copy(block[:], buf)
	return block
}

func TestAltEndorsementDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultAltEndorsementEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := AltEndorsement{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultAltEndorsement.ID, decoded.ID)
	assert.Equal(defaultAltEndorsement.EndorsedHash, decoded.EndorsedHash)
	assert.Equal(defaultAltEndorsement.ContainingHash, decoded.ContainingHash)
	assert.Equal(defaultAltEndorsement.BlockOfProof, decoded.BlockOfProof)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestAltEndorsementSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultAltEndorsement.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultAltEndorsementEncoded, blockEncoded)
}

func TestAltEndorsementRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultAltEndorsementEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := AltEndorsement{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultAltEndorsement.ID, decoded.ID)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultAltEndorsementEncoded, blockReEncoded)
}
