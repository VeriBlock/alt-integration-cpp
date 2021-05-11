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
	defaultVbkMerklePathEncoded = "04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c0400000002200000000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000000"
	defaultVbkMerklePathLayers  = [][32]byte{
		parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
		parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
	}
	defaultVbkMerklePathSubject   = "1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"
	defaultVbkMerklePathTreeIndex = int32(1)
	defaultVbkMerklePathIndex     = int32(0)
)

func TestVbkMerklePathDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	merklePath := parseHex(defaultVbkMerklePathEncoded)
	stream := bytes.NewReader(merklePath)
	decoded := VbkMerklePath{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkMerklePathTreeIndex, decoded.TreeIndex)
	assert.Equal(defaultVbkMerklePathIndex, decoded.Index)
	assert.Equal(defaultVbkMerklePathSubject, hex.EncodeToString(decoded.Subject[:]))
	assert.Equal(defaultVbkMerklePathLayers, decoded.Layers)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkMerklePathSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	subject := parse32Bytes(defaultVbkMerklePathSubject)
	path := VbkMerklePath{defaultVbkMerklePathTreeIndex, defaultVbkMerklePathIndex, subject, defaultVbkMerklePathLayers}
	stream := new(bytes.Buffer)
	assert.NoError(path.ToVbkEncoding(stream))
	pathEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultVbkMerklePathEncoded, pathEncoded)
}

func TestVbkMerklePathRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	merklePath := parseHex(defaultVbkMerklePathEncoded)
	stream := bytes.NewReader(merklePath)
	decoded := VbkMerklePath{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkMerklePathIndex, decoded.Index)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	pathReEncoded := hex.EncodeToString(outputStream.Bytes())

	assert.Equal(defaultVbkMerklePathEncoded, pathReEncoded)
}
