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
	err := btcTx.ToVbkEncoding(stream)
	assert.NoError(err)
	decoded, err := BtcTxFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(btcTx.Tx, decoded.Tx)
	assert.Equal(0, stream.Len(), "Stream has more data")
}
