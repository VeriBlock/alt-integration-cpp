package entities

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io"
	"unsafe"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
)

// MerklePath ...
type MerklePath struct {
	Index   int32
	Subject [32]byte
	Layers  [][32]byte
}

// ToVbkEncoding ...
func (v *MerklePath) ToVbkEncoding(stream io.Writer) error {
	blockStream := new(bytes.Buffer)
	if err := v.ToRaw(blockStream); err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, blockStream.Bytes())
}

// ToRaw ...
func (v *MerklePath) ToRaw(stream io.Writer) error {
	if err := veriblock.WriteSingleFixedBEValue(stream, v.Index); err != nil {
		return err
	}
	if err := veriblock.WriteSingleFixedBEValue(stream, int32(len(v.Layers))); err != nil {
		return err
	}
	subjectSizeBytes, err := veriblock.FixedArray(int32(len(v.Subject)))
	if err != nil {
		return err
	}
	if err := veriblock.WriteSingleFixedBEValue(stream, int32(len(subjectSizeBytes))); err != nil {
		return err
	}
	if _, err := stream.Write(subjectSizeBytes); err != nil {
		return err
	}
	for _, layer := range v.Layers {
		if err := veriblock.WriteSingleByteLenValue(stream, layer[:]); err != nil {
			return err
		}
	}
	return nil
}

// FromVbkEncoding ...
func (v *MerklePath) FromVbkEncoding(stream io.Reader, subject [32]byte) error {
	merkleBytes, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxMerkleBytes)
	if err != nil {
		return err
	}
	merkleStream := bytes.NewReader(merkleBytes)
	return v.FromRaw(merkleStream, subject)
}

// FromRaw ...
func (v *MerklePath) FromRaw(stream io.Reader, subject [32]byte) error {
	if err := veriblock.ReadSingleBEValue(stream, &v.Index); err != nil {
		return err
	}
	var numLayers int32
	if err := veriblock.ReadSingleBEValue(stream, &numLayers); err != nil {
		return err
	}
	if err := veriblock.CheckRange(int64(numLayers), 0, veriblock.MaxLayerCountMerkle); err != nil {
		return err
	}
	var sizeOfSizeBottomData int32
	if err := veriblock.ReadSingleBEValue(stream, &sizeOfSizeBottomData); err != nil {
		return err
	}
	if uintptr(sizeOfSizeBottomData) != unsafe.Sizeof(sizeOfSizeBottomData) {
		return errors.New("merkle-size-of-size-range")
	}
	var sizeOfBottomData int32
	if err := binary.Read(stream, binary.BigEndian, &sizeOfBottomData); err != nil {
		return err
	}
	if sizeOfBottomData != veriblock.Sha256HashSize {
		return errors.New("merkle-size-of-data-range")
	}
	v.Layers = make([][32]byte, numLayers)
	for i := int32(0); i < numLayers; i++ {
		arr, err := veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
		if err != nil {
			return err
		}
		copy(v.Layers[i][:], arr)
	}
	v.Subject = subject
	return nil
}
