package entities

import (
	"encoding/binary"
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// BtcBlockAddon ...
type BtcBlockAddon struct {
	Refs []int32
}

// ToRaw ...
func (v *BtcBlockAddon) ToRaw(stream io.Writer) error {
	return veriblock.WriteArrayOf(stream, v.Refs, func(w io.Writer, val interface{}) error {
		return binary.Write(w, binary.BigEndian, val)
	})
}

// FromRaw ...
func (v *BtcBlockAddon) FromRaw(stream io.Reader) error {
	refs, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, func(r io.Reader) (interface{}, error) {
		var res int32
		if err := binary.Read(r, binary.BigEndian, &res); err != nil {
			return nil, err
		}
		return &res, nil
	})
	if err != nil {
		return err
	}
	v.Refs = make([]int32, len(refs))
	for i, ref := range refs {
		v.Refs[i] = *ref.(*int32)
	}
	return nil
}
