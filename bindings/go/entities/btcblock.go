package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// BtcHash is 32 byte hash of BtcBlock
type BtcHash [32]byte

// UnmarshalJSON parses a hash in hex syntax.
func (h *BtcHash) UnmarshalJSON(input []byte) error {
	return veriblock.UnmarshalJSON(input, h[:])
}

// BtcBlock ...
type BtcBlock struct {
	Version       uint32
	PreviousBlock [32]byte
	MerkleRoot    [32]byte
	Timestamp     uint32
	Bits          uint32
	Nonce         uint32
}

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

// FromVbkEncoding ...
func (v *BtcBlock) FromVbkEncoding(stream io.Reader) error {
	blockBytes, err := veriblock.ReadSingleByteLenValue(stream, veriblock.BtcHeaderSize, veriblock.BtcHeaderSize)
	if err != nil {
		return err
	}
	blockStream := bytes.NewReader(blockBytes)
	return v.FromRaw(blockStream)
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
