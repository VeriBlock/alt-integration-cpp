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
	defaultVbkEndorsement = VbkEndorsement{
		parse32Bytes("f7de2995898800ab109af96779b979a60715da9bf2bbb745b300000000000000"),
		parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
		parse24Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1"),
		parse32Bytes("f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"),
	}
	defaultVbkEndorsementEncoded = "20f7de2995898800ab109af96779b979a60715da9bf2bbb745b30000000000000018f85486026bf4ead8a37a42925332ec8b553f8e310974fea118f85486026bf4ead8a37a42925332ec8b553f8e310974fea120f85486026bf4ead8a37a42925332ec8b553f8e310974fea1eba238f7cee6165e"
)

func TestVbkEndorsementDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkEndorsementEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := VbkEndorsement{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkEndorsement.ID, decoded.ID)
	assert.Equal(defaultVbkEndorsement.EndorsedHash, decoded.EndorsedHash)
	assert.Equal(defaultVbkEndorsement.ContainingHash, decoded.ContainingHash)
	assert.Equal(defaultVbkEndorsement.BlockOfProof, decoded.BlockOfProof)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkEndorsementSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultVbkEndorsement.ToVbkEncoding(stream))
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkEndorsementEncoded, blockEncoded)
}

func TestVbkEndorsementRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	blockEncoded := parseHex(defaultVbkEndorsementEncoded)
	stream := bytes.NewReader(blockEncoded)
	decoded := VbkEndorsement{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkEndorsement.ID, decoded.ID)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	blockReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultVbkEndorsementEncoded, blockReEncoded)
}
