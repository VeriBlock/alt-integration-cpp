package entities

import (
	"bytes"
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

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

// GetID - returns id of VBKBlock
func (v *VbkBlock) GetID() ([12]byte, error) {
	hash, err := v.GetHash()
	if err != nil {
		return [12]byte{}, err
	}
	var res [12]byte
	copy(res[:], hash[:])
	return res, nil
}

// GetHash - returns progPowHash of VBKBlock
func (v *VbkBlock) GetHash() ([24]byte, error) {
	blockStream := new(bytes.Buffer)
	if err := v.ToRaw(blockStream); err != nil {
		return [24]byte{}, err
	}
	var res [24]byte
	// TODO: Get progPowHash
	copy(res[:], blockStream.Bytes())
	return res, nil
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
