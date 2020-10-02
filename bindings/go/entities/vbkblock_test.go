package entities

import (
	"bytes"
	"encoding/hex"
	"testing"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
	"github.com/stretchr/testify/assert"
)

var (
	defaultBlock = VbkBlock{
		5000,
		2,
		[12]byte{68, 156, 96, 97, 146, 148, 84, 106, 216, 37, 175, 3},
		[9]byte{176, 147, 86, 55, 134, 6, 121, 221, 213},
		[9]byte{94, 228, 253, 33, 8, 46, 24, 104, 110},
		[16]byte{38, 187, 253, 167, 213, 228, 70, 46, 242, 74, 224, 45, 103, 228, 125, 120},
		1553699059,
		16842752,
		123317,
	}
	defaultBlockEncoded = "41000013880002449c60619294546ad825af03b0935637860679ddd55ee4fd21082e18686e26bbfda7d5e4462ef24ae02d67e47d785c9b90f301010000000001e1b5"
)

func TestDeserialize(t *testing.T) {
	assert := assert.New(t)

	vbkblock := veriblock.Parse(defaultBlockEncoded)
	stream := bytes.NewReader(vbkblock)
	block, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)

	assert.Equal(defaultBlock.Height, block.Height)
	assert.Equal(defaultBlock.Version, block.Version)
	assert.Equal(hex.EncodeToString(defaultBlock.PreviousBlock[:]), hex.EncodeToString(block.PreviousBlock[:]))
	assert.Equal(hex.EncodeToString(defaultBlock.PreviousKeystone[:]), hex.EncodeToString(block.PreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultBlock.SecondPreviousKeystone[:]), hex.EncodeToString(block.SecondPreviousKeystone[:]))
	assert.Equal(hex.EncodeToString(defaultBlock.MerkleRoot[:]), hex.EncodeToString(block.MerkleRoot[:]))
	assert.Equal(defaultBlock.Timestamp, block.Timestamp)
	assert.Equal(defaultBlock.Difficulty, block.Difficulty)
	assert.Equal(defaultBlock.Nonce, block.Nonce)
}

func TestSerialize(t *testing.T) {
	assert := assert.New(t)

	stream := new(bytes.Buffer)
	err := defaultBlock.ToVbkEncoding(stream)
	assert.NoError(err)
	blockEncoded := hex.EncodeToString(stream.Bytes())
	assert.Equal(defaultBlockEncoded, blockEncoded)
}

func TestRoundTrip(t *testing.T) {
	assert := assert.New(t)

	blockEncoded, err := hex.DecodeString(defaultBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(blockEncoded)
	decoded, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)
	assert.Equal(defaultBlock.Version, decoded.Version)

	outputStream := new(bytes.Buffer)
	err = decoded.ToVbkEncoding(outputStream)
	assert.NoError(err)
	assert.Equal(defaultBlockEncoded, hex.EncodeToString(outputStream.Bytes()))
}

func TestGetBlockHash(t *testing.T) {
	assert := assert.New(t)

	block := VbkBlock{}
	block.Height = 5000
	block.Version = 2
	copy(block.PreviousBlock[:], veriblock.Parse("94E7DC3E3BE21A96ECCF0FBD"))
	copy(block.PreviousKeystone[:], veriblock.Parse("F5F62A3331DC995C36"))
	copy(block.SecondPreviousKeystone[:], veriblock.Parse("B0935637860679DDD5"))
	copy(block.MerkleRoot[:], veriblock.Parse("DB0F135312B2C27867C9A83EF1B99B98"))
	block.Timestamp = 1553699987
	block.Difficulty = 117586646
	block.Nonce = 1924857207

	hash, err := block.GetHash()
	assert.NoError(err)
	assert.NotEqual(veriblock.Parse("00000000000000001f45c91342b8ac0ea7ae4d721be2445dc86ddc3f0e454f60"), hash[:])
}

func TestGetID(t *testing.T) {
	assert := assert.New(t)

	atvBytes, err := hex.DecodeString(defaultBlockEncoded)
	assert.NoError(err)
	stream := bytes.NewReader(atvBytes)
	vbkblock, err := VbkBlockFromVbkEncoding(stream)
	assert.NoError(err)
	id, err := vbkblock.GetID()
	assert.NoError(err)
	assert.NotEqual("cd97599e23096ad42f119b5a", hex.EncodeToString(id[:]))
}
