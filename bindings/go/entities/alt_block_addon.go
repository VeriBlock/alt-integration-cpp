package entities

import (
	"bytes"
	"io"
	"math"

	veriblock "github.com/VeriBlock/alt-integration-cpp/bindings/go"
)

// AltBlockAddon ...
type AltBlockAddon struct {
	popState PopState
	// List of changes introduced in this block
	AtvIDs      []AtvID
	VtbIDs      []VtbID
	VbkBlockIDs []VbkID
}

// ToRaw ...
func (v *AltBlockAddon) ToRaw(stream io.Writer) error {
	if err := v.popState.ToRaw(stream); err != nil {
		return err
	}
	if err := veriblock.WriteArrayOf(stream, v.AtvIDs, veriblock.WriteSingleByteLenValue); err != nil {
		return err
	}
	if err := veriblock.WriteArrayOf(stream, v.VtbIDs, veriblock.WriteSingleByteLenValue); err != nil {
		return err
	}
	return veriblock.WriteArrayOf(stream, v.VbkBlockIDs, veriblock.WriteSingleByteLenValue)
}

// ToRawBytes ...
func (v *AltBlockAddon) ToRawBytes() ([]byte, error) {
	var buffer bytes.Buffer
	err := v.ToRaw(&buffer)
	return buffer.Bytes(), err
}

// FromRaw ...
func (v *AltBlockAddon) FromRaw(stream io.Reader) error {
	readFunc := func(r io.Reader) (interface{}, error) { return veriblock.ReadSingleByteLenValueDefault(r) }
	v.popState.IsAlt = true
	if err := v.popState.FromRaw(stream); err != nil {
		return err
	}
	atvids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.AtvIDs = make([]AtvID, len(atvids))
	for i, atvid := range atvids {
		copy(v.AtvIDs[i][:], atvid.([]byte))
	}
	vtbids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.VtbIDs = make([]VtbID, len(vtbids))
	for i, vtbid := range vtbids {
		copy(v.VtbIDs[i][:], vtbid.([]byte))
	}
	vbkblockids, err := veriblock.ReadArrayOf(stream, 0, math.MaxInt32, readFunc)
	if err != nil {
		return err
	}
	v.VbkBlockIDs = make([]VbkID, len(vbkblockids))
	for i, vbkblockid := range vbkblockids {
		copy(v.VbkBlockIDs[i][:], vbkblockid.([]byte))
	}
	return nil
}

// FromRawBytes ...
func (v *AltBlockAddon) FromRawBytes(data []byte) error {
	buffer := bytes.NewBuffer(data)
	return v.FromRaw(buffer)
}
