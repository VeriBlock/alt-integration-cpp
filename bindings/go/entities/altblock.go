package entities

import (
	"encoding/binary"
	"io"
)

// AltBlock ...
type AltBlock struct {
	Hash          []byte
	PreviousBlock []byte
	Height        int32
	Timestamp     uint32
}

// ToVbkEncoding ...
func (v *AltBlock) ToVbkEncoding(stream io.Writer) error {
	err := binary.Write(stream, binary.BigEndian, uint32(len(v.Hash)))
	if err != nil {
		return err
	}
	for i := 0; i < len(v.Hash); i++ {
		err = binary.Write(stream, binary.BigEndian, v.Hash[i])
		if err != nil {
			return err
		}
	}
	err = binary.Write(stream, binary.BigEndian, uint32(len(v.PreviousBlock)))
	if err != nil {
		return err
	}
	for i := 0; i < len(v.PreviousBlock); i++ {
		err = binary.Write(stream, binary.BigEndian, v.PreviousBlock[i])
		if err != nil {
			return err
		}
	}
	err = binary.Write(stream, binary.BigEndian, v.Height)
	if err != nil {
		return err
	}
	return binary.Write(stream, binary.BigEndian, v.Timestamp)
}

// ToRaw ...
func (v *AltBlock) ToRaw(stream io.Writer) error {
	return v.ToVbkEncoding(stream)
}

// AltBlockFromVbkEncoding ...
func AltBlockFromVbkEncoding(stream io.Reader) (*AltBlock, error) {
	block := AltBlock{}
	var hashSize uint32
	err := binary.Read(stream, binary.BigEndian, &hashSize)
	if err != nil {
		return nil, err
	}
	block.Hash = make([]byte, hashSize)
	for i := uint32(0); i < hashSize; i++ {
		err = binary.Read(stream, binary.BigEndian, &block.Hash[i])
		if err != nil {
			return nil, err
		}
	}
	err = binary.Read(stream, binary.BigEndian, &hashSize)
	if err != nil {
		return nil, err
	}
	block.PreviousBlock = make([]byte, hashSize)
	for i := uint32(0); i < hashSize; i++ {
		err = binary.Read(stream, binary.BigEndian, &block.PreviousBlock[i])
		if err != nil {
			return nil, err
		}
	}
	err = binary.Read(stream, binary.BigEndian, &block.Height)
	if err != nil {
		return nil, err
	}
	err = binary.Read(stream, binary.BigEndian, &block.Timestamp)
	if err != nil {
		return nil, err
	}
	return &block, nil
}

// AltBlockFromRaw ...
func AltBlockFromRaw(stream io.Reader) (*AltBlock, error) {
	return AltBlockFromVbkEncoding(stream)
}
