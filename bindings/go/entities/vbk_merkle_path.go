package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// VbkMerklePath ...
type VbkMerklePath struct {
	// An indentifier of which transactions tree (pop=0, normal=1)
	TreeIndex int32
	// The index of the bottom data TxID in the block it came from
	Index int32
	// TxID that this merkle path authenticates
	Subject [32]byte
	// The layers in the merkle path
	Layers [][32]byte
}

// ToVbkEncoding ...
func (v *VbkMerklePath) ToVbkEncoding(stream io.Writer) error {
	if err := veriblock.WriteSingleFixedBEValue(stream, v.TreeIndex); err != nil {
		return err
	}
	if err := veriblock.WriteSingleFixedBEValue(stream, v.Index); err != nil {
		return err
	}
	if err := veriblock.WriteSingleByteLenValue(stream, v.Subject[:]); err != nil {
		return err
	}
	if err := veriblock.WriteSingleFixedBEValue(stream, int32(len(v.Layers))); err != nil {
		return err
	}
	for _, layer := range v.Layers {
		if err := veriblock.WriteSingleByteLenValue(stream, layer); err != nil {
			return err
		}
	}
	return nil
}

// FromVbkEncoding ...
func (v *VbkMerklePath) FromVbkEncoding(stream io.Reader) error {
	if err := veriblock.ReadSingleBEValue(stream, &v.TreeIndex); err != nil {
		return err
	}
	if err := veriblock.ReadSingleBEValue(stream, &v.Index); err != nil {
		return err
	}
	subject, err := veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
	if err != nil {
		return err
	}
	copy(v.Subject[:], subject)
	layers, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxLayerCountMerkle, func(stream io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
	})
	if err != nil {
		return err
	}
	v.Layers = make([][32]byte, len(layers))
	for i, layer := range layers {
		copy(v.Layers[i][:], layer.([]byte))
	}
	return nil
}
