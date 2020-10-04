package entities

import (
	"io"

	veriblock "github.com/VeriBlock/alt-integration-cpp"
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
	err := veriblock.WriteSingleFixedBEValue(stream, v.TreeIndex)
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleFixedBEValue(stream, v.Index)
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleByteLenValue(stream, v.Subject[:])
	if err != nil {
		return err
	}
	err = veriblock.WriteSingleFixedBEValue(stream, int32(len(v.Layers)))
	if err != nil {
		return err
	}
	for _, layer := range v.Layers {
		err = veriblock.WriteSingleByteLenValue(stream, layer)
		if err != nil {
			return err
		}
	}
	return nil
}

// VbkMerklePathFromVbkEncoding ...
func VbkMerklePathFromVbkEncoding(stream io.Reader) (*VbkMerklePath, error) {
	path := VbkMerklePath{}
	err := veriblock.ReadSingleBEValue(stream, &path.TreeIndex)
	if err != nil {
		return nil, err
	}
	err = veriblock.ReadSingleBEValue(stream, &path.Index)
	if err != nil {
		return nil, err
	}
	subject, err := veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
	if err != nil {
		return nil, err
	}
	copy(path.Subject[:], subject)
	layers, err := veriblock.ReadArrayOf(stream, 0, veriblock.MaxLayerCountMerkle, func(stream io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValue(stream, veriblock.Sha256HashSize, veriblock.Sha256HashSize)
	})
	if err != nil {
		return nil, err
	}
	path.Layers = make([][32]byte, len(layers))
	for i, layer := range layers {
		copy(path.Layers[i][:], layer.([]byte))
	}
	return &path, nil
}
