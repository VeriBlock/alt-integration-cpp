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
	defaultPathEncoded = "02019f040000067b040000000c040000000400000020204d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f61220023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af120096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead202f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb572093e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a275620f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c200824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d7420b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162201732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17202d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb9920dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618202cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42"
	defaultLayers      = [][32]byte{
		parse32Bytes("4d66077fdf24246ffd6b6979dfedef5d46588654addeb35edb11e993c131f612"),
		parse32Bytes("023d1abe8758c6f917ec0c65674bbd43d66ee14dc667b3117dfc44690c6f5af1"),
		parse32Bytes("096ddba03ca952af133fb06307c24171e53bf50ab76f1edeabde5e99f78d4ead"),
		parse32Bytes("2f32cf1bee50349d56fc1943af84f2d2abda520f64dc4db37b2f3db20b0ecb57"),
		parse32Bytes("93e70120f1b539d0c1495b368061129f30d35f9e436f32d69967ae86031a2756"),
		parse32Bytes("f554378a116e2142f9f6315a38b19bd8a1b2e6dc31201f2d37a058f03c39c06c"),
		parse32Bytes("0824705685ceca003c95140434ee9d8bbbf4474b83fd4ecc2766137db9a44d74"),
		parse32Bytes("b7b9e52f3ee8ce4fbb8be7d6cf66d33a20293f806c69385136662a74453fb162"),
		parse32Bytes("1732c9a35e80d4796babea76aace50b49f6079ea3e349f026b4491cfe720ad17"),
		parse32Bytes("2d9b57e92ab51fe28a587050fd82abb30abd699a5ce8b54e7cd49b2a827bcb99"),
		parse32Bytes("dcba229acdc6b7f028ba756fd5abbfebd31b4227cd4137d728ec5ea56c457618"),
		parse32Bytes("2cf1439a6dbcc1a35e96574bddbf2c5db9174af5ad0d278fe92e06e4ac349a42"),
	}
	defaultSubject = "94e097b110ba3adbb7b6c4c599d31d675de7be6e722407410c08ef352be585f1"
	defaultIndex   = int32(1659)
)

func TestMerklePathDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	merklePath := parseHex(defaultPathEncoded)
	subject := parse32Bytes(defaultSubject)
	stream := bytes.NewReader(merklePath)
	decoded := MerklePath{}
	assert.NoError(decoded.FromVbkEncoding(stream, subject))

	assert.Equal(defaultIndex, decoded.Index)
	assert.Equal(defaultSubject, hex.EncodeToString(decoded.Subject[:]))
	assert.Equal(defaultLayers, decoded.Layers)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestMerklePathSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	subject := parse32Bytes(defaultSubject)
	path := MerklePath{defaultIndex, subject, defaultLayers}
	stream := new(bytes.Buffer)
	assert.NoError(path.ToVbkEncoding(stream))
	pathEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultPathEncoded, pathEncoded)
}

func TestMerklePathRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	merklePath := parseHex(defaultPathEncoded)
	subject := parse32Bytes(defaultSubject)
	stream := bytes.NewReader(merklePath)
	decoded := MerklePath{}
	assert.NoError(decoded.FromVbkEncoding(stream, subject))
	assert.Equal(defaultIndex, decoded.Index)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToVbkEncoding(outputStream))
	pathReEncoded := hex.EncodeToString(outputStream.Bytes())
	assert.Equal(defaultPathEncoded, pathReEncoded)
}
