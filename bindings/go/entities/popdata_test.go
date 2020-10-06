package entities

import (
	"bytes"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
	"github.com/stretchr/testify/assert"
)

func TestPopDataRoundTrip(t *testing.T) {
	assert := assert.New(t)

	atvBytes := veriblock.Parse(defaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	atv, err := AtvFromVbkEncoding(stream)
	assert.NoError(err)

	vtbBytes := veriblock.Parse(defaultVtbEncoded)
	stream = bytes.NewReader(vtbBytes)
	vtb, err := VtbFromVbkEncoding(stream)
	assert.NoError(err)

	expectedPopData := PopData{1, []VbkBlock{}, []Vtb{*vtb}, []Atv{*atv}}
	outputStream := new(bytes.Buffer)
	err = expectedPopData.ToVbkEncoding(outputStream)
	assert.NoError(err)

	decodedPopData, err := PopDataFromVbkEncoding(outputStream)
	assert.Equal(expectedPopData, *decodedPopData)
}
