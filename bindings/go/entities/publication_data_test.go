package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	defaultPublicationEncoded = "0100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f206279746573"
	defaultPublication        = PublicationData{
		0,
		[]byte("header bytes"),
		[]byte("payout info bytes"),
		[]byte("context info bytes"),
	}
)

func TestPublicationDataDeserialize(t *testing.T) {
	assert := assert.New(t)

	pub := parseHex(defaultPublicationEncoded)
	stream := bytes.NewReader(pub)
	decoded := PublicationData{}
	assert.NoError(decoded.FromRaw(stream))

	assert.Equal(defaultPublication.Identifier, decoded.Identifier)
	assert.Equal(defaultPublication.Header, decoded.Header)
	assert.Equal(defaultPublication.PayoutInfo, decoded.PayoutInfo)
	assert.Equal(defaultPublication.ContextInfo, decoded.ContextInfo)

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestPublicationDataSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(defaultPublication.ToRaw(stream))
	assert.Equal(defaultPublicationEncoded, hex.EncodeToString(stream.Bytes()))
}

func TestPublicationDataRoundTrip(t *testing.T) {
	assert := assert.New(t)

	pub := parseHex(defaultPublicationEncoded)
	stream := bytes.NewReader(pub)
	decoded := PublicationData{}
	assert.NoError(decoded.FromRaw(stream))
	assert.Equal(defaultPublication.Identifier, decoded.Identifier)

	outputStream := new(bytes.Buffer)
	assert.NoError(decoded.ToRaw(outputStream))
	assert.Equal(defaultPublicationEncoded, hex.EncodeToString(outputStream.Bytes()))
}
