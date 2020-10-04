package entities

import (
	"bytes"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
	"github.com/stretchr/testify/assert"
)

var (
	outputBytes      = veriblock.Parse("01166772F51AB208D32771AB1506970EEB664462730B838E020539")
	outputAddress, _ = AddressFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	outputValue      = Output{*outputAddress, Coin{1337}}
)

func TestOutputDeserialize(t *testing.T) {
	assert := assert.New(t)

	stream := bytes.NewReader(outputBytes)
	output, err := OutputFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(outputValue, *output)
	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestOutputSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	outputValue.ToVbkEncoding(stream)
	assert.Equal(outputBytes, stream.Bytes())
}

func TestOutputRoundTrip(t *testing.T) {
	assert := assert.New(t)

	stream := bytes.NewReader(outputBytes)
	output, err := OutputFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(outputValue, *output)

	outputStream := new(bytes.Buffer)
	output.ToVbkEncoding(outputStream)
	assert.Equal(outputBytes, outputStream.Bytes())
}
