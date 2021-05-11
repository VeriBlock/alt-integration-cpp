// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/stretchr/testify/assert"
)

var (
	publicationData = PublicationData{0, []byte{}, []byte{}, []byte{}}
	emptyBytes64    = make([]byte, 64)
	vbktxAddress1   = addressFromString("V8dy5tWcP7y36kxiJwxKPKUrWAJbjs")
	vbktxAddress2   = addressFromString("V7GghFKRA6BKqtHD7LTdT2ao93DRNA")
	defaultVbkTx    = VbkTx{
		veriblock.NetworkBytePair{
			HasNetworkByte: false,
			NetworkByte:    0,
			TypeID:         uint8(veriblock.TxTypeVbkTx),
		},
		vbktxAddress1,
		Coin{3500000000},
		[]Output{{
			vbktxAddress2,
			Coin{3499999999},
		}},
		5904,
		publicationData,
		emptyBytes64,
		emptyBytes64,
	}
	defaultVbkTxEncoded = "014901011667a654ee3e0c918d8652b63829d7f3bef98524bf899604d09dc30001011667901a1e11c650509efc46e09e81678054d8562af02b04d09dc2ff0217100108010001000100010040000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
)

func TestVbkTxDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	vbktx := parseHex(defaultVbkTxEncoded)
	stream := bytes.NewReader(vbktx)
	decoded := VbkTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(defaultVbkTx.NetworkOrType.TypeID, decoded.NetworkOrType.TypeID)
	assert.Equal(defaultVbkTx.SourceAddress, decoded.SourceAddress)
	assert.Equal(defaultVbkTx.SourceAmount, decoded.SourceAmount)
	assert.Equal(defaultVbkTx.SignatureIndex, decoded.SignatureIndex)
	assert.Equal(len(defaultVbkTx.Outputs), len(decoded.Outputs))
	assert.Equal(defaultVbkTx.Outputs[0], decoded.Outputs[0])
	assert.Equal(defaultVbkTx.PublicationData.Identifier, decoded.PublicationData.Identifier)
	assert.Equal(defaultVbkTx.Signature, decoded.Signature)
	assert.Equal(defaultVbkTx.PublicKey, decoded.PublicKey)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestVbkTxSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	err := defaultVbkTx.ToVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(defaultVbkTxEncoded, hex.EncodeToString(stream.Bytes()))
}

func TestVbkTxRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	txEncoded := parseHex(defaultVbkTxEncoded)
	stream := bytes.NewReader(txEncoded)
	decoded := VbkTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(defaultVbkTx.SignatureIndex, decoded.SignatureIndex)

	outputStream := new(bytes.Buffer)
	assert.NoError(defaultVbkTx.ToVbkEncoding(outputStream))
	assert.Equal(defaultVbkTxEncoded, hex.EncodeToString(outputStream.Bytes()))
}
