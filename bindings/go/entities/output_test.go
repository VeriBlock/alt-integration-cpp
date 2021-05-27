// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	outputBytes   = parseHex("01166772F51AB208D32771AB1506970EEB664462730B838E020539")
	outputAddress = addressFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	outputValue   = Output{outputAddress, Coin{1337}}
)

func TestOutputDeserialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(outputBytes)
	output := Output{}
	assert.NoError(output.FromVbkEncoding(stream))
	assert.Equal(outputValue, output)
	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestOutputSerialize(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(outputValue.ToVbkEncoding(stream))
	assert.Equal(outputBytes, stream.Bytes())
}

func TestOutputRoundTrip(t *testing.T) {
	t.Parallel()

	assert := assert.New(t)

	stream := bytes.NewReader(outputBytes)
	output := Output{}
	assert.NoError(output.FromVbkEncoding(stream))
	assert.Equal(outputValue, output)

	outputStream := new(bytes.Buffer)
	assert.NoError(output.ToVbkEncoding(outputStream))
	assert.Equal(outputBytes, outputStream.Bytes())
}
