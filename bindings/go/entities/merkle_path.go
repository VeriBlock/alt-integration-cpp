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
	err := v.ToRaw(blockStream)
	if err != nil {
		return err
	}
	return veriblock.WriteVarLenValue(stream, blockStream.Bytes())
}

// ToRaw ...
func (v *MerklePath) ToRaw(stream io.Writer) error {
	err := veriblock.WriteSingleFixedBEValue(stream, v.Index)
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleFixedBEValue(stream, int32(len(v.Layers)))
	if err != nil {
		return err
	}
	subjectSizeBytes, err := veriblock.FixedArray(int32(len(v.Subject)))
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleFixedBEValue(stream, int32(len(subjectSizeBytes)))
	if err != nil {
		return err
	}
	_, err = stream.Write(subjectSizeBytes)
	if err != nil {
		return err
	}
	for _, layer := range v.Layers {
		err = veriblock.WriteSingleByteLenValue(stream, layer[:])
		if err != nil {
			return err
		}
	}
	return nil
}

// MerklePathFromVbkEncoding ...
func MerklePathFromVbkEncoding(stream io.Reader, subject [32]byte) (*MerklePath, error) {
	merkleBytes, err := veriblock.ReadVarLenValue(stream, 0, veriblock.MaxMerkleBytes)
	if err != nil {
		return nil, err
	}
	merkleStream := bytes.NewReader(merkleBytes)
	return MerklePathFromRaw(merkleStream, subject)
}

// MerklePathFromRaw ...
func MerklePathFromRaw(stream io.Reader, subject [32]byte) (*MerklePath, error) {
	path := &MerklePath{}
	err := veriblock.ReadSingleBEValue(stream, &path.Index)
	if err != nil {
		return nil, err
	}
	var numLayers int32
	err = veriblock.ReadSingleBEValue(stream, &numLayers)
	if err != nil {
		return nil, err
	}
	err = veriblock.CheckRange(int64(numLayers), 0, veriblock.MaxLayerCountMerkle)
	if err != nil {
		return nil, err
	}
	var sizeOfSizeBottomData int32
	err = veriblock.ReadSingleBEValue(stream, &sizeOfSizeBottomData)
	if err != nil {
		return nil, err
	}
	if uintptr(sizeOfSizeBottomData) != unsafe.Sizeof(sizeOfSizeBottomData) {
		return nil, errors.New("merkle-size-of-size-range")
	}
	var sizeOfBottomData int32
	err = binary.Read(stream, binary.BigEndian, &sizeOfBottomData)
	if err != nil {
		return nil, err
	}
	if sizeOfBottomData != veriblock.Sha256HashSize {
		return nil, errors.New("merkle-size-of-data-range")
	}
	path.Layers = make([][32]byte, numLayers)
	for i := int32(0); i < numLayers; i++ {
		arr, err := veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
		if err != nil {
			return nil, err
		}
		copy(path.Layers[i][:], arr)
	}
	path.Subject = subject
	return path, nil
}
