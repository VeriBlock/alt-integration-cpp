package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
	"github.com/VeriBlock/alt-integration-cpp/bindings/go/ffi"
)

// VbkID is 12 byte ID of VbkBlock
type VbkID [12]byte

// VbkHash is 24 byte hash of VbkBlock
type VbkHash [24]byte

// VbkBlock ...
type VbkBlock struct {
	Height                 int32
	Version                int16
	PreviousBlock          [12]byte
	PreviousKeystone       [9]byte
	SecondPreviousKeystone [9]byte
	MerkleRoot             [16]byte
	Timestamp              int32
	Difficulty             int32
	Nonce                  uint64
}

// GetHash ...
func (v *VbkBlock) GetHash() VbkHash {
	buffer := new(bytes.Buffer)
	v.ToVbkEncoding(buffer)
	return ffi.VbkBlockGetHash(buffer.Bytes())
}

// GetGenericHash ...
func (v *VbkBlock) GetGenericHash() []byte {
	hash := v.GetHash()
	return hash[:]
}

// GetBlockTime ...
func (v *VbkBlock) GetBlockTime() uint32 {
	return uint32(v.Timestamp)
}

// GetDifficulty ...
func (v *VbkBlock) GetDifficulty() uint32 {
	return uint32(v.Difficulty)
}

// GetID - Returns id of VBKBlock
func (v *VbkBlock) GetID() VbkID {
	buffer := new(bytes.Buffer)
	v.ToVbkEncoding(buffer)
	return ffi.VbkBlockGetID(buffer.Bytes())
}

// ToVbkEncoding ...
func (v *VbkBlock) ToVbkEncoding(stream io.Writer) error {
	blockStream := new(bytes.Buffer)
	if err := v.ToRaw(blockStream); err != nil {
		return err
	}
	return veriblock.WriteSingleByteLenValue(stream, blockStream.Bytes())
}

// ToRaw ...
func (v *VbkBlock) ToRaw(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.Height); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, v.Version); err != nil {
		return err
	}
	if _, err := stream.Write(v.PreviousBlock[:]); err != nil {
		return err
	}
	if _, err := stream.Write(v.PreviousKeystone[:]); err != nil {
		return err
	}
	if _, err := stream.Write(v.SecondPreviousKeystone[:]); err != nil {
		return err
	}
	if _, err := stream.Write(v.MerkleRoot[:]); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, v.Timestamp); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, v.Difficulty); err != nil {
		return err
	}
	nonce := make([]byte, 5)
	nonce[4] = byte(v.Nonce)
	nonce[3] = byte(v.Nonce >> 8)
	nonce[2] = byte(v.Nonce >> 16)
	nonce[1] = byte(v.Nonce >> 24)
	nonce[0] = byte(v.Nonce >> 32)
	return binary.Write(stream, binary.BigEndian, nonce)
}

// FromVbkEncoding ...
func (v *VbkBlock) FromVbkEncoding(stream io.Reader) error {
	blockBytes, err := veriblock.ReadSingleByteLenValue(stream, veriblock.VbkHeaderSizeProgpow, veriblock.VbkHeaderSizeProgpow)
	if err != nil {
		return err
	}
	blockStream := bytes.NewReader(blockBytes)
	return v.FromRaw(blockStream)
}

// FromRaw ...
func (v *VbkBlock) FromRaw(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.Height); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Version); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.PreviousBlock); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.PreviousKeystone); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.SecondPreviousKeystone); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.MerkleRoot); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Timestamp); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Difficulty); err != nil {
		return err
	}
	b := make([]byte, 5)
	if _, err := stream.Read(b); err != nil {
		return err
	}
	v.Nonce = uint64(b[0])<<32 | uint64(b[1])<<24 | uint64(b[2])<<16 | uint64(b[3])<<8 | uint64(b[4])
	return nil
}

// ToJSON ...
func (v *VbkBlock) ToJSON() (map[string]interface{}, error) {
	id := v.GetID()
	hash := v.GetHash()
	res := map[string]interface{}{
		"id":                     hex.EncodeToString(id[:]),
		"hash":                   hex.EncodeToString(hash[:]),
		"height":                 v.Height,
		"version":                v.Version,
		"previousBlock":          hex.EncodeToString(v.PreviousBlock[:]),
		"previousKeystone":       hex.EncodeToString(v.PreviousKeystone[:]),
		"secondPreviousKeystone": hex.EncodeToString(v.SecondPreviousKeystone[:]),
		"merkleRoot":             hex.EncodeToString(v.MerkleRoot[:]),
		"timestamp":              v.Timestamp,
		"difficulty":             v.Difficulty,
		"nonce":                  v.Nonce,
	}
	return res, nil
}
