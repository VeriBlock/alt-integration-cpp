package entities

import (
	"encoding/binary"
	"encoding/hex"
	"io"
)

// AltBlock ...
type AltBlock struct {
	Hash          []byte
	PreviousBlock []byte
	Height        int32
	Timestamp     uint32
}

// GetHash ...
func (v *AltBlock) GetHash() []byte {
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
	if err := binary.Write(stream, binary.BigEndian, uint32(len(v.Hash))); err != nil {
		return err
	}
	for i := 0; i < len(v.Hash); i++ {
		if err := binary.Write(stream, binary.BigEndian, v.Hash[i]); err != nil {
			return err
		}
	}
	if err := binary.Write(stream, binary.BigEndian, uint32(len(v.PreviousBlock))); err != nil {
		return err
	}
	for i := 0; i < len(v.PreviousBlock); i++ {
		if err := binary.Write(stream, binary.BigEndian, v.PreviousBlock[i]); err != nil {
			return err
		}
	}
	if err := binary.Write(stream, binary.BigEndian, v.Height); err != nil {
		return err
	}
	return binary.Write(stream, binary.BigEndian, v.Timestamp)
}

// ToRaw ...
func (v *AltBlock) ToRaw(stream io.Writer) error {
	return v.ToVbkEncoding(stream)
}

// FromVbkEncoding ...
func (v *AltBlock) FromVbkEncoding(stream io.Reader) error {
	var hashSize uint32
	if err := binary.Read(stream, binary.BigEndian, &hashSize); err != nil {
		return err
	}
	v.Hash = make([]byte, hashSize)
	for i := uint32(0); i < hashSize; i++ {
		if err := binary.Read(stream, binary.BigEndian, &v.Hash[i]); err != nil {
			return err
		}
	}
	if err := binary.Read(stream, binary.BigEndian, &hashSize); err != nil {
		return err
	}
	v.PreviousBlock = make([]byte, hashSize)
	for i := uint32(0); i < hashSize; i++ {
		if err := binary.Read(stream, binary.BigEndian, &v.PreviousBlock[i]); err != nil {
			return err
		}
	}
	if err := binary.Read(stream, binary.BigEndian, &v.Height); err != nil {
		return err
	}
	return binary.Read(stream, binary.BigEndian, &v.Timestamp)
}

// FromRaw ...
func (v *AltBlock) FromRaw(stream io.Reader) error {
	return v.FromVbkEncoding(stream)
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
