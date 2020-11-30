package entities

import (
	"bytes"
	"encoding/binary"
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// VbkBlockAddon ...
type VbkBlockAddon struct {
	popState PopState
	// Reference counter for fork resolution
	RefCount uint32
	// List of changes introduced in this block
	VtbIDs []VtbID
}

// ToRaw ...
func (v *VbkBlockAddon) ToRaw(stream io.Writer) error {
	if err := binary.Write(stream, binary.BigEndian, v.RefCount); err != nil {
		return err
	}
	if err := v.popState.ToRaw(stream); err != nil {
		return err
	}
	return veriblock.WriteArrayOf(stream, v.VtbIDs, veriblock.WriteSingleByteLenValue)
}

// ToRawBytes ...
func (v *VbkBlockAddon) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromRaw ...
func (v *VbkBlockAddon) FromRaw(stream io.Reader) error {
	if err := binary.Read(stream, binary.BigEndian, &v.RefCount); err != nil {
		return err
	}
	if err := v.popState.FromRaw(stream); err != nil {
		return err
	}
	vtbids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, func(r io.Reader) (interface{}, error) {
		return veriblock.ReadSingleByteLenValueDefault(r)
	})
	if err != nil {
		return err
	}
	v.VtbIDs = make([]VtbID, len(vtbids))
	for i, vtbid := range vtbids {
		copy(v.VtbIDs[i][:], vtbid.([]byte))
	}
	return nil
}

// FromRawBytes ...
func (v *VbkBlockAddon) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}
