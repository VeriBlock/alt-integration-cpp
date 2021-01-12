package entities

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// AltBlockName ...
const AltBlockName = "ALT"

// AltHash is dynamic byte hash of ALT blocks
type AltHash []byte

// ContextInfoContainerHash ...
type ContextInfoContainerHash [veriblock.Sha256HashSize]byte

// Bytes ...
func (v *AltHash) Bytes() []byte {
	return *v
}

// Bytes ...
func (v *ContextInfoContainerHash) Bytes() []byte {
	return v[:]
}

// AltBlock ...
type AltBlock struct {
	Hash          AltHash
	PreviousBlock AltHash
	Height        int32
	Timestamp     uint32
}

// Name ...
func (v *AltBlock) Name() string { return AltBlockName }

// GetHash ...
func (v *AltBlock) GetHash() AltHash {
	return v.Hash
}

// GetGenericHash ...
func (v *AltBlock) GetGenericHash() []byte {
	return v.Hash
}

// GetBlockTime ...
func (v *AltBlock) GetBlockTime() uint32 {
	return v.Timestamp
}

// GetDifficulty ...
func (v *AltBlock) GetDifficulty() uint32 {
	return 0
}

// ToVbkEncoding ...
func (v *AltBlock) ToVbkEncoding(stream io.Writer) error {
	if err := veriblock.WriteSingleByteLenValue(stream, v.Hash); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.PreviousBlock); err != nil {
		return err
	}
	if err := binary.Write(stream, binary.BigEndian, v.Height); err != nil {
		return err
	}
	return binary.Write(stream, binary.BigEndian, v.Timestamp)
}

// ToVbkEncodingBytes ...
func (v *AltBlock) ToVbkEncodingBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToVbkEncoding(&buffer)
	return buffer.Bytes(), err
}

// ToRaw ...
func (v *AltBlock) ToRaw(stream io.Writer) error {
	return v.ToVbkEncoding(stream)
}

// ToRawBytes ...
func (v *AltBlock) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromVbkEncoding ...
func (v *AltBlock) FromVbkEncoding(stream io.Reader) error {
	var err error
	if v.Hash, err = veriblock.ReadSingleByteLenValue(stream, veriblock.MinAltHashSize, veriblock.MaxAltHashSize); err != nil {
		return err
	}
	if v.PreviousBlock, err = veriblock.ReadSingleByteLenValue(stream, veriblock.MinAltHashSize, veriblock.MaxAltHashSize); err != nil {
		return err
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Height); err != nil {
		return err
	}
	return binary.Read(stream, binary.BigEndian, &v.Timestamp)
}

// FromVbkEncodingBytes ...
func (v *AltBlock) FromVbkEncodingBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromVbkEncoding(buffer)
}

// FromRaw ...
func (v *AltBlock) FromRaw(stream io.Reader) error {
	return v.FromVbkEncoding(stream)
}

// FromRawBytes ...
func (v *AltBlock) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}

// ToJSON ...
func (v *AltBlock) ToJSON() (map[string]interface{}, error) {
	res := map[string]interface{}{
		"hash":          hex.EncodeToString(v.GetHash()),
		"previousBlock": hex.EncodeToString(v.PreviousBlock),
		"height":        v.Height,
		"timestamp":     v.Timestamp,
	}
	return res, nil
}
