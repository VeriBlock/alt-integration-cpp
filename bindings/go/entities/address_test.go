package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

// Standard address
var (
	AddressValue = "V5Ujv72h4jEBcKnALGc4fKqs6CDAPX"
	AddressBytes = parseHex("01166772F51AB208D32771AB1506970EEB664462730B838E")
)

func TestAddressDeserialize(t *testing.T) {
	assert := assert.New(t)

	stream := bytes.NewReader(AddressBytes)
	address, err := AddressFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(AddressValue, address.ToString())
	assert.Equal(AddressTypeStandard, address.GetType())

	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestAddressSerialize(t *testing.T) {
	assert := assert.New(t)

	address, err := AddressFromString(AddressValue)
	assert.NoError(err)
	stream := new(bytes.Buffer)

	address.ToVbkEncoding(stream)

	assert.Equal(AddressBytes, stream.Bytes())
}

func TestAddressRoundTrip(t *testing.T) {
	assert := assert.New(t)

	stream := bytes.NewReader(AddressBytes)
	decoded, err := AddressFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(AddressValue, decoded.ToString())

	outputStream := new(bytes.Buffer)
	decoded.ToVbkEncoding(outputStream)
	assert.Equal(AddressBytes, outputStream.Bytes())
}

func TestAddressValidStandard(t *testing.T) {
	assert := assert.New(t)

	addressString := "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d"

	address, err := AddressFromString(addressString)
	assert.NoError(err)
	assert.Equal(AddressTypeStandard, address.GetType())
	assert.Equal(addressString, address.ToString())
}

func TestAddressValidMultisig(t *testing.T) {
	assert := assert.New(t)

	addressString := "V23Cuyc34u5rdk9psJ86aFcwhB1md0"

	address, err := AddressFromString(addressString)
	assert.NoError(err)
	assert.Equal(AddressTypeMultisig, address.GetType())
	assert.Equal(addressString, address.ToString())
}

func TestAddressDerivedFromPublicKey(t *testing.T) {
	assert := assert.New(t)

	publicKey := parseHex("3056301006072a8648ce3d020106052b8104000a03420004cb427e41a0114874080a4b1e2ab7920e22cd2d188c87140defa447ee5fc44bb848e1c0db5ef206de2e7002f6c86952be4823a4c08e65e4cdbeb904a8b95763aa")
	addressString := "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d"
	address, err := AddressFromString(addressString)
	assert.NoError(err)
	assert.True(address.IsDerivedFromPublicKey(publicKey))
}

func TestAddressNotDerivedFromPublicKey(t *testing.T) {
	assert := assert.New(t)

	publicKey := parseHex("3056301006072a8648ce3d020106052b8104000a03420004cb427e41a0114874080a4b1e2ab7920e22cd2d188c87140defa447ee5fc44bb848e1c0db5ef206de2e7002f6c86952be4823a4c08e65e4cdbeb904a8b95763aa")
	addressString := "V23Cuyc34u5rdk9psJ86aFcwhB1md0"
	address, err := AddressFromString(addressString)
	assert.NoError(err)
	assert.False(address.IsDerivedFromPublicKey(publicKey))
}

func TestAddressParseStandard(t *testing.T) {
	assert := assert.New(t)

	addressString := "VFFDWUMLJwLRuNzH4NX8Rm32E59n6d"
	address, err := AddressFromString(addressString)
	assert.NoError(err)
	outputStream := new(bytes.Buffer)
	address.ToVbkEncoding(outputStream)
	stream := bytes.NewReader(outputStream.Bytes())
	decoded, err := AddressFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(addressString, decoded.ToString())
	assert.Equal(AddressTypeStandard, decoded.GetType())
	assert.Equal(0, stream.Len(), "Stream has more data")
}

func TestAddressParseMultisig(t *testing.T) {
	assert := assert.New(t)

	addressString := "V23Cuyc34u5rdk9psJ86aFcwhB1md0"
	address, err := AddressFromString(addressString)
	assert.NoError(err)
	outputStream := new(bytes.Buffer)
	address.ToVbkEncoding(outputStream)
	stream := bytes.NewReader(outputStream.Bytes())
	decoded, err := AddressFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(addressString, decoded.ToString())
	assert.Equal(AddressTypeMultisig, decoded.GetType())
	assert.Equal(0, stream.Len(), "Stream has more data")
}
