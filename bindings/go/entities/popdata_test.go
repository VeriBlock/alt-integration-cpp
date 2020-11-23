package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestPopDataRoundTrip(t *testing.T) {
	assert := assert.New(t)

	atvBytes := parseHex(defaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	atv := Atv{}
	assert.NoError(atv.FromVbkEncoding(stream))

	vtbBytes := parseHex(defaultVtbEncoded)
	stream = bytes.NewReader(vtbBytes)
	vtb := Vtb{}
	assert.NoError(vtb.FromVbkEncoding(stream))

	expectedPopData := PopData{1, []VbkBlock{}, []Vtb{vtb}, []Atv{atv}}
	outputStream := new(bytes.Buffer)
	assert.NoError(expectedPopData.ToVbkEncoding(outputStream))

	decodedPopData := PopData{}
	assert.NoError(decodedPopData.FromVbkEncoding(outputStream))
	assert.Equal(expectedPopData, decodedPopData)
}
