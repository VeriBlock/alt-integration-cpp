package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestCoinRound(t *testing.T) {
	assert := assert.New(t)

	coin := Coin{1}
	stream := new(bytes.Buffer)
	err := coin.ToVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal([]byte{1, 1}, stream.Bytes())
	decoded, err := CoinFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(coin.Units, decoded.Units)
	assert.Equal(0, stream.Len(), "Stream has more data")
}
