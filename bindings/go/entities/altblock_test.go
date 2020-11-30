package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

var defaultAltBlock = AltBlock{
	parseHex("0c0102030405060708090a0b0c0c0000000000000000000000000000000100000064"),
	[]byte{},
	156,
	1466,
}

func TestAltBlockRoundTrip(t *testing.T) {
	assert := assert.New(t)

	outputStream := new(bytes.Buffer)
	assert.NoError(defaultAltBlock.ToVbkEncoding(outputStream))
	stream := bytes.NewReader(outputStream.Bytes())
	deserializedBlock := AltBlock{}
	assert.NoError(deserializedBlock.FromVbkEncoding(stream))

	assert.Equal(defaultAltBlock.Hash, deserializedBlock.Hash)
	assert.Equal(defaultAltBlock.Height, deserializedBlock.Height)
	assert.Equal(defaultAltBlock.Timestamp, deserializedBlock.Timestamp)
}
