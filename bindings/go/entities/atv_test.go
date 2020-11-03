package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/stretchr/testify/assert"
)

var (
	publicationAtvData  = PublicationData{0, []byte("header bytes"), []byte("payout info bytes"), []byte("context info bytes")}
	defaultAtvSignature = parseHex("30440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7")
	defaultAtvPublicKey = parseHex("3056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e")
	atvAddress          = addressFromString("V5Ujv72h4jEBcKnALGc4fKqs6CDAPX")
	defaultAtvTx        = VbkTx{
		veriblock.NetworkBytePair{
			HasNetworkByte: false,
			NetworkByte:    0,
			TypeID:         uint8(veriblock.TxTypeVbkTx),
		},
		atvAddress,
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
	DefaultAtv        = Atv{1, defaultAtvTx, defaultAtvPath, defaultAtvVbkBlock}
	DefaultAtvEncoded = "0000000101580101166772f51ab208d32771ab1506970eeb664462730b838e0203e800010701370100010c6865616465722062797465730112636f6e7465787420696e666f20627974657301117061796f757420696e666f2062797465734630440220398b74708dc8f8aee68fce0c47b8959e6fce6354665da3ed87a83f708e62aa6b02202e6c00c00487763c55e92c7b8e1dd538b7375d8df2b2117e75acbb9db7deb3c7583056301006072a8648ce3d020106052b8104000a03420004de4ee8300c3cd99e913536cf53c4add179f048f8fe90e5adf3ed19668dd1dbf6c2d8e692b1d36eac7187950620a28838da60a8c9dd60190c14c59b82cb90319e04000000010400000000201fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c040000000220000000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000041000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f3010100000000000001"
)

func TestAtvDeserialize(t *testing.T) {
	assert := assert.New(t)

	atvBytes := parseHex(DefaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	decoded := Atv{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(atvAddress, decoded.Transaction.SourceAddress)
}

func TestAtvSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	assert.NoError(DefaultAtv.ToVbkEncoding(stream))
	assert.Equal(DefaultAtvEncoded, hex.EncodeToString(stream.Bytes()))
}

func TestAtvRoundTrip(t *testing.T) {
	assert := assert.New(t)

	atvBytes := parseHex(DefaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	decoded := Atv{}
	assert.NoError(decoded.FromVbkEncoding(stream))
	assert.Equal(atvAddress, decoded.Transaction.SourceAddress)

	outputStream := new(bytes.Buffer)
	assert.NoError(DefaultAtv.ToVbkEncoding(outputStream))
	assert.Equal(DefaultAtvEncoded, hex.EncodeToString(outputStream.Bytes()))
}

func TestAtvGetId(t *testing.T) {
	assert := assert.New(t)

	atvBytes := parseHex(DefaultAtvEncoded)
	stream := bytes.NewReader(atvBytes)
	atv := Atv{}
	assert.NoError(atv.FromVbkEncoding(stream))

	id := atv.GetID()
	assert.Equal("c6d96b8e87f3e347aa1d1051bb3af39c8ea60612ced905d11c6f92d7b6bd50f5", hex.EncodeToString(id[:]))
}
