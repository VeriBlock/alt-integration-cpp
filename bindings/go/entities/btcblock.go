package entities

import (
	"bytes"
	"encoding/binary"
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// BtcBlock ...
type BtcBlock struct {
	Version       uint32
	PreviousBlock [32]byte
	MerkleRoot    [32]byte
	Timestamp     uint32
	Bits          uint32
	Nonce         uint32
}

// ToVbkEncoding ...
func (v *BtcBlock) ToVbkEncoding(stream io.Writer) error {
	blockStream := new(bytes.Buffer)
	err := v.ToRaw(blockStream)
	if err != nil {
		return err
	}
	return veriblock.WriteSingleByteLenValue(stream, blockStream.Bytes())
}

// ToRaw ...
func (v *BtcBlock) ToRaw(stream io.Writer) error {
	err := binary.Write(stream, binary.LittleEndian, v.Version)
	if err != nil {
		return err
	}
	_, err = stream.Write(veriblock.ReverseBytes(v.PreviousBlock[:]))
	if err != nil {
		return err
	}
	_, err = stream.Write(veriblock.ReverseBytes(v.MerkleRoot[:]))
	if err != nil {
		return err
	}
	err = binary.Write(stream, binary.LittleEndian, v.Timestamp)
	if err != nil {
		return err
	}
	err = binary.Write(stream, binary.LittleEndian, v.Bits)
	if err != nil {
		return err
	}
	err = binary.Write(stream, binary.LittleEndian, v.Nonce)
	if err != nil {
		return err
	}
	return nil
}

// BtcBlockFromVbkEncoding ...
func BtcBlockFromVbkEncoding(stream io.Reader) (*BtcBlock, error) {
	blockBytes, err := veriblock.ReadSingleByteLenValue(stream, veriblock.BtcHeaderSize, veriblock.BtcHeaderSize)
	if err != nil {
		return nil, err
	}
	blockStream := bytes.NewReader(blockBytes)
	return BtcBlockFromRaw(blockStream)
}

// BtcBlockFromRaw ...
func BtcBlockFromRaw(stream io.Reader) (*BtcBlock, error) {
	block := &BtcBlock{}
	err := binary.Read(stream, binary.LittleEndian, &block.Version)
	if err != nil {
		return nil, err
	}
	_, err = stream.Read(block.PreviousBlock[:])
	if err != nil {
		return nil, err
	}
	copy(block.PreviousBlock[:], veriblock.ReverseBytes(block.PreviousBlock[:]))
	_, err = stream.Read(block.MerkleRoot[:])
	if err != nil {
		return nil, err
	}
	copy(block.MerkleRoot[:], veriblock.ReverseBytes(block.MerkleRoot[:]))
	err = binary.Read(stream, binary.LittleEndian, &block.Timestamp)
	if err != nil {
		return nil, err
	}
	err = binary.Read(stream, binary.LittleEndian, &block.Bits)
	if err != nil {
		return nil, err
	}
	err = binary.Read(stream, binary.LittleEndian, &block.Nonce)
	if err != nil {
		return nil, err
	}
	return block, nil
}
