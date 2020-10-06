package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
	"github.com/stretchr/testify/assert"
)

var (
	publicationAtvData  = PublicationData{0, []byte("header bytes"), []byte("payout info bytes"), []byte("context info bytes")}
	defaultAtvSignature = veriblock.Parse("30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7")
	defaultAtvPublicKey = veriblock.Parse("3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e")
	atvAddress, _       = AddressFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	defaultAtvTx        = VbkTx{
		veriblock.NetworkBytePair{
			HasNetworkByte: false,
			NetworkByte:    0,
			TypeID:         uint8(veriblock.TxTypeVbkTx),
		},
		*atvAddress,
		Coin{1000},
		[]Output{},
		7,
		publicationAtvData,
		defaultAtvSignature,
		defaultAtvPublicKey,
	}
	defaultAtvPath = VbkMerklePath{
		1,
		0,
		parse32Bytes("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"),
		[][32]byte{
			parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
			parse32Bytes("0000000000000000000000000000000000000000000000000000000000000000"),
		},
	}
	defaultAtvVbkBlock = VbkBlock{
		5000,
		2,
		parse12Bytes("449c60619294546ad825af03"),
		parse9Bytes("b0935637860679ddd5"),
		parse9Bytes("5ee4fd21082e18686e"),
		parse16Bytes("26bbfda7d5e4462ef24ae02d67e47d78"),
		1553699059,
		16842752,
		1,
	}
	defaultAtv        = Atv{1, defaultAtvTx, defaultAtvPath, defaultAtvVbkBlock}
	defaultAtvEncoded = "0000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
)

func TestAtvDeserialize(t *testing.T) {
	assert := assert.New(t)

	atvBytes := veriblock.Parse(defaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	decoded, err := AtvFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(*atvAddress, decoded.Transaction.SourceAddress)
}

func TestAtvSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	err := defaultAtv.ToVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(defaultAtvEncoded, hex.EncodeToString(stream.Bytes()))
}

func TestAtvRoundTrip(t *testing.T) {
	assert := assert.New(t)

	atvBytes := veriblock.Parse(defaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	decoded, err := AtvFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(*atvAddress, decoded.Transaction.SourceAddress)

	outputStream := new(bytes.Buffer)
	err = defaultAtv.ToVbkEncoding(outputStream)
	assert.NoError(err)
	assert.Equal(defaultAtvEncoded, hex.EncodeToString(outputStream.Bytes()))
}
