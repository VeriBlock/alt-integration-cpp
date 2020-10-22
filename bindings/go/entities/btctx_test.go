package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestBtcTxRound(t *testing.T) {
	assert := assert.New(t)

	btcTx := BtcTx{[]byte{1, 2, 3}}
	stream := new(bytes.Buffer)
	assert.NoError(btcTx.ToVbkEncoding(stream))
	decoded := BtcTx{}
	assert.NoError(decoded.FromVbkEncoding(stream))

	assert.Equal(btcTx.Tx, decoded.Tx)
	assert.Equal(0, stream.Len(), "Stream has more data")
}
