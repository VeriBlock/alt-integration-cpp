package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// BtcBlockName ...
const BtcBlockName = "BTC"

// BtcHash is 32 byte hash of BtcBlock
type BtcHash [veriblock.Sha256HashSize]byte

// Bytes ...
func (v *BtcHash) Bytes() []byte {
	return v[:]
}

// UnmarshalJSON parses a hash in hex syntax.
func (h *BtcHash) UnmarshalJSON(input []byte) error {
	return veriblock.UnmarshalJSON(input, h[:])
}

// BtcBlock ...
type BtcBlock struct {
	Version       uint32
	PreviousBlock [veriblock.Sha256HashSize]byte
	MerkleRoot    [veriblock.Sha256HashSize]byte
	Timestamp     uint32
	Bits          uint32
	Nonce         uint32
}

// Name ...
func (v *BtcBlock) Name() string { return BtcBlockName }

// GetHash ...
func (v *BtcBlock) GetHash() BtcHash {
	buffer := new(bytes.Buffer)
	v.ToVbkEncoding(buffer)
	return ffi.BtcBlockGetHash(buffer.Bytes())
}

// GetGenericHash ...
func (v *BtcBlock) GetGenericHash() []byte {
	hash := v.GetHash()
	return hash[:]
}

// GetBlockTime ...
func (v *BtcBlock) GetBlockTime() uint32 {
	return v.Timestamp
}

// GetDifficulty ...
func (v *BtcBlock) GetDifficulty() uint32 {
	return v.Bits
}

// ToVbkEncoding ...
func (v *BtcBlock) ToVbkEncoding(stream io.Writer) error {
	blockStream := new(bytes.Buffer)
	if err := v.ToRaw(blockStream); err != nil {
		return err
	}
	return veriblock.WriteSingleByteLenValue(stream, blockStream.Bytes())
}

// ToVbkEncodingBytes ...
func (v *BtcBlock) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// ToRaw ...
func (v *BtcBlock) ToRaw(stream io.Writer) error {
	if err := binary.Write(stream, binary.LittleEndian, v.Version); err != nil {
		return err
	}
	if _, err := stream.Write(veriblock.ReverseBytes(v.PreviousBlock[:])); err != nil {
		return err
	}
	if _, err := stream.Write(veriblock.ReverseBytes(v.MerkleRoot[:])); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.LittleEndian, v.Timestamp); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.LittleEndian, v.Bits); err != nil {
		return err
	}
	return binary.Write(stream, binary.LittleEndian, v.Nonce)
}

// ToRawBytes ...
func (v *BtcBlock) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *BtcBlock) FromVbkEncoding(stream io.Reader) error {
	blockBytes, err := veriblock.ReadSingleByteLenValue(stream, veriblock.BtcHeaderSize, veriblock.BtcHeaderSize)
	if err != nil {
		return err
	}
	blockStream := bytes.NewReader(blockBytes)
	return v.FromRaw(blockStream)
}

// FromVbkEncodingBytes ...
func (v *BtcBlock) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// FromRaw ...
func (v *BtcBlock) FromRaw(stream io.Reader) error {
	if err := binary.Read(stream, binary.LittleEndian, &v.Version); err != nil {
		return err
	}
	if _, err := stream.Read(v.PreviousBlock[:]); err != nil {
		return err
	}
	copy(v.PreviousBlock[:], veriblock.ReverseBytes(v.PreviousBlock[:]))
	if _, err := stream.Read(v.MerkleRoot[:]); err != nil {
		return err
	}
	copy(v.MerkleRoot[:], veriblock.ReverseBytes(v.MerkleRoot[:]))
	if err := binary.Read(stream, binary.LittleEndian, &v.Timestamp); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.LittleEndian, &v.Bits); err != nil {
		return err
	}
	return binary.Read(stream, binary.LittleEndian, &v.Nonce)
}

// FromRawBytes ...
func (v *BtcBlock) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}

// ToJSON ...
func (v *BtcBlock) ToJSON() (map[string]interface{}, error) {
	hash := v.GetHash()
	res := map[string]interface{}{
		"hash":          hex.EncodeToString(hash[:]),
		"version":       v.Version,
		"previousBlock": hex.EncodeToString(v.PreviousBlock[:]),
		"merkleRoot":    hex.EncodeToString(v.MerkleRoot[:]),
		"timestamp":     v.Timestamp,
		"bits":          v.Bits,
		"nonce":         v.Nonce,
	}
	return res, nil
}
